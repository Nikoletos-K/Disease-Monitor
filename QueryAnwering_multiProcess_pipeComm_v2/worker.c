#include "patientData.h"


RBTNode ** inodeRBTree;
RBTNode ** RBTreeArray;
char ***filesArray;
char * pipename[2];
int bufferSize;
char **dirsArray;
int numOfDirs;
volatile sig_atomic_t SIGUSR1_flag=0;
volatile sig_atomic_t SIGINT_flag=0;
volatile sig_atomic_t workerEnds;


int main(int argc,char **argv){

	int i=1;
	char * pipeRead = malloc(1);
	char size[50];
	int bytesRead,sizeofReads=0,fd=-1,ReadfromParent=1,index=0,packageSize = -1;
	initializeDataStructures();

	/* ------------- Signal masks ---------------------*/

	static struct sigaction actSIGUSR1,actSIGINT;
	actSIGUSR1.sa_handler = catchSIGUSR1;
	actSIGINT.sa_handler = catchSIGINT_OR_SIGQUIT;

	sigfillset(&(actSIGUSR1.sa_mask));
	sigfillset(&(actSIGINT.sa_mask));

	sigaction(SIGUSR1,&actSIGUSR1,NULL);
	sigaction(SIGINT,&actSIGINT,NULL);
	sigaction(SIGQUIT,&actSIGINT,NULL);

	while(argv[i]!=NULL){
		/*- reading command-line arguments -*/
		if(!strcmp(argv[i],"-pc"))
			pipename[CHILD] = strdup(argv[i+1]);
		else if(!strcmp(argv[i],"-pp"))
			pipename[PARENT] = strdup(argv[i+1]);
		else if(!strcmp(argv[i],"-b"))
			bufferSize = atoi(argv[i+1]);
		i++;			
	}

	if((fd=open(pipename[PARENT],O_RDONLY|O_NONBLOCK))<0)	// Opening pipe for reading subdirectories
		fprintf(stderr,"Worker %ld: Pipe %s didn't open\n",(long)getpid(),pipename[PARENT]);

	/* --------------- Reading from pipe ------------------- */ 
	while(ReadfromParent){

		char buffer[bufferSize];
		bytesRead=read(fd,buffer,bufferSize);
		switch(bytesRead){
			case -1:
				if(errno == EAGAIN)
					break;
				if(errno == EINTR)
					printf("STOP\n");
				break;
			case 0:	break;
			default:
				pipeRead = realloc(pipeRead,(sizeofReads+bytesRead)*sizeof(char));
				// printf("r %d\n", sizeofReads+bytesRead);
				strncpy(pipeRead+sizeofReads,buffer,bytesRead);
				sizeofReads += bytesRead;
				break;
		}

		if(packageSize != -1 && sizeofReads>=packageSize+index+1)
			break;
				
		if(packageSize==-1 && bytesRead>0){

			while(index<sizeofReads && pipeRead[index]!='%' ){
				size[index] = pipeRead[index];
				index++;
			}
			if(index<sizeofReads && pipeRead[index]=='%'){
				size[index] = '\0';
				packageSize = atoi(size);

				if(sizeofReads>=packageSize+index+1)
					break;
			}
		}
	}
	// close(fd);

	/* ------------ Opening subdirs and reading records for each subdir ------------- */
	dirsArray = malloc(sizeof(char*));
	char *token;

	/* ------ Split strings to eash subdir --------- */
	token = strtok(pipeRead,"%");
	while(token!=NULL){
		token = strtok(NULL,"%");
		if(token == NULL)
			break;
		dirsArray = realloc(dirsArray,(numOfDirs+1)*sizeof(char*));
		dirsArray[numOfDirs] = strdup(token);
		numOfDirs++;
	}
	filesArray = malloc(numOfDirs*sizeof(char**)); 
	RBTreeArray = malloc(sizeof(RBTNode*)*numOfDirs);
	int * numOfFiles = malloc(numOfDirs*sizeof(int));
	inodeRBTree = malloc(sizeof(RBTNode*)*numOfDirs);
	for(int d=0;d<numOfDirs;d++){
		RBTreeArray[d] = RBTConstruct();
		inodeRBTree[d] = RBTConstruct();
	}

	/*---------- Informing Data Structures and calculating summary statistics ------- */	

	CalculateAndSendStatistics(INITIAL_FILES,numOfFiles);

	while(!workerEnds){

		if(SIGUSR1_flag!=0){
			if(CalculateAndSendStatistics(NEW_FILES,numOfFiles))
				kill(getppid(),SIGUSR1);
			else
				printf("No new files to any directory\n");
			SIGUSR1_flag = 0;
			printf("\n> ");
			fflush(stdout);
		}

		if(SIGINT_flag){
			char filename[50];
			sprintf(filename,"log_file.%ld",(long)getpid());
			FILE * log = fopen(filename,"w+");
			for(int i=0;i<numOfDirs;i++){
				char country[50];
				getCountry(dirsArray[i],country);
				fprintf(log,"%s\n",country);
				fflush(log);
			}
			workerEnds=1;
			fclose(log);
		}
	}

	for(int d=0;d<numOfDirs;d++){
		for(int f=0;f<numOfFiles[d];f++){
			free(filesArray[d][f]);
		}
		free(dirsArray[d]);
		free(filesArray[d]);
		RBTDestroyTreeAndData(RBTreeArray[d],destroyPatient);
		RBTDestroyAllNodes(inodeRBTree[d]);		
	}
	free(filesArray);
	free(dirsArray);
	free(RBTreeArray);
	free(inodeRBTree);
	free(numOfFiles);
	free(pipeRead);
	free(pipename[PARENT]);
	free(pipename[CHILD]);
	destroyDataStructures();
	kill(getppid(),SIGCHLD);
	printf("Worker %ld ended \n",(long)getpid());
	return 0;
}

int CalculateAndSendStatistics(int newfiles,int *numOfFiles){

	DIR * input_dir;
	struct dirent * direntp;
	int fd = -1,numOfnewFiles[numOfDirs],f;
	char ***statistics_toPipe = malloc(numOfDirs*sizeof(char**));

	for(int d=0;d<numOfDirs;d++){
		input_dir = opendir(dirsArray[d]);
		if(input_dir==NULL)
			fprintf(stderr,"Worker %ld : opendir %s\n",(long)getpid(),strerror(errno));
		
		numOfnewFiles[d]=0;
		RBTNode * tempTree;
		if(newfiles){
			tempTree = RBTConstruct();
			printf("-- %d\n",numOfFiles[d] );
			for(int f=0;f<numOfFiles[d];f++){
				
				RBTInsert(&tempTree,(void*)filesArray[d][f],(void*)filesArray[d][f],stringComparator);

			}	
			f=numOfFiles[d];
		}else{
			f=0;
			filesArray[d] = malloc(sizeof(char*));
		}
	

		while((direntp=readdir(input_dir))!=NULL){
			if(strcmp(direntp->d_name,".") && strcmp(direntp->d_name,"..")){
				if(RBTFindNode(inodeRBTree[d],(void *)direntp->d_name,stringComparator)==NULL){

					filesArray[d] = realloc(filesArray[d],(f+1)*sizeof(char*));
					filesArray[d][f] = strdup(direntp->d_name);
					RBTInsert(&inodeRBTree[d],(void *)direntp->d_name,(void *)direntp->d_name,stringComparator);
					printf("%s\n",filesArray[d][f] );
					f++;
					if(newfiles)
						numOfnewFiles[d]++;
				}else
					printf("File exists\n");
				
			}
		}
		// RBTPrintTree(inodeRBTree[d],printString);
		if(newfiles)
			numOfFiles[d] += numOfnewFiles[d];
		else
			numOfFiles[d] = f;


		if(newfiles && numOfnewFiles[d]==0){
			statistics_toPipe[d]=NULL;
			RBTDestroyAllNodes(tempTree);
			closedir(input_dir);
			continue;
		}else{
			printf("numOfFiles %d\n",numOfFiles[d] );
			qsort(filesArray[d],numOfFiles[d],sizeof(char*),dateCompare);
			if(newfiles)
				statistics_toPipe[d] = malloc(sizeof(char*)*numOfnewFiles[d]);
			else
				statistics_toPipe[d] = malloc(sizeof(char*)*numOfFiles[d]);
		}

		int statistics_index=0;
		for(int i=0;i<numOfFiles[d];i++){

			if(newfiles && RBTFindNode(tempTree,(void *)filesArray[d][i],stringComparator)!=NULL){
				printf("Found in tree %s \n",filesArray[d][i]);
				continue;
			}

			char filePath[PATH_MAX];
			strcpy(filePath,dirsArray[d]);
			strcat(filePath,"/");
			strcat(filePath,filesArray[d][i]);
			FILE * tempfile;
			
			if((tempfile = fopen(filePath,"r"))==NULL){
				fprintf(stderr,"File %s can not open : %s\n",filePath,strerror(errno));
				exit(1);
			}

			char country[100],temp[100];
			temp[0] = '\0';
			strcat(temp,dirsArray[d]);
			getCountry(temp,country);
			printf("%s\n",country );

			HashTable * diseaseHT_perDay = create_HashTable(100,516);
			
			statistics_toPipe[d][statistics_index] = malloc((strlen(country)+strlen(filesArray[d][i])+3)*sizeof(char));
			strcpy(statistics_toPipe[d][statistics_index],"");
			strcat(statistics_toPipe[d][statistics_index],filesArray[d][i]);
			strcat(statistics_toPipe[d][statistics_index],"%");
			strcat(statistics_toPipe[d][statistics_index],country);
			strcat(statistics_toPipe[d][statistics_index],"%");
			
			while(!feof(tempfile)){
				char id[100],fname[100],lname[100],enterORexit[100],disease[100];int age;
				int elems = fscanf(tempfile,"%s %s %s %s %s %d \n",id,enterORexit,fname,lname,disease,&age);
				if(elems < 6 || !onlyLettersandNumbers(id) || !(!strcmp(enterORexit,"ENTER") || !strcmp(enterORexit,"EXIT"))){
					fprintf(stderr,"ERROR\n");
					continue;
				}
				if(!strcmp(enterORexit,"ENTER")){
					if(RBTFindNode(RBTreeArray[d],(void*)id,stringComparator)==NULL){
						Patient * p = createPatient(id,fname,lname,disease,age,filesArray[d][i],country);
						RBTInsert(&RBTreeArray[d],(void *) p,p->id,stringComparator);
						insert_toHashTable(p,p->disease,p->id,diseaseHT_perDay,stringComparator);
					}
					else
						fprintf(stderr,"ERROR\n");
				}else{
					RBTNode * temp;
					if((temp=RBTFindNode(RBTreeArray[d],(void*)id,stringComparator))!=NULL){
						Patient * p = (Patient*) get_RBTData(temp);
						if(dateCompare(&p->entryDate,&(filesArray[d][i]))<=0)
							infromExitDate(p,filesArray[d][i]);
						else
							fprintf(stderr,"ERROR\n");
					}else
						fprintf(stderr,"ERROR\n");
				}
			}

			statistics_toPipe[d][statistics_index] = sendSummaryStatistics(diseaseHT_perDay,statistics_toPipe[d][statistics_index]);			
			destroyHashTable(diseaseHT_perDay);
			fclose(tempfile);
			statistics_index++;
		}
		if(newfiles)
			RBTDestroyAllNodes(tempTree);
		closedir(input_dir);
	}

	if(newfiles){
		int dd=0;
		while(dd<numOfDirs && numOfnewFiles[dd]==0) dd++;
		if(dd==numOfDirs){
			for(int d=0;d<numOfDirs;d++){
				if(statistics_toPipe[d]==NULL) continue;
				for(int f=0;f<numOfnewFiles[d];f++){
					if(statistics_toPipe[d][f]!=NULL)
						free(statistics_toPipe[d][f]);
				}
				if(statistics_toPipe[d]!=NULL)
					free(statistics_toPipe[d]);
			}
			free(statistics_toPipe);
			return 0;
		}
	}
	while(1){
		fd=open(pipename[CHILD],O_WRONLY);
		if(fd<0){
			if(errno==ENXIO)
				continue;
			else
				fprintf(stderr,"Pipe %s didn't open : %s \n",pipename[CHILD],strerror(errno));
		}else{
			break;
		}
	}

	char * message;
	if(newfiles)
		message = concat_toMessage(statistics_toPipe,numOfDirs,numOfnewFiles);		
	else
		message = concat_toMessage(statistics_toPipe,numOfDirs,numOfFiles);

	message = insertSizetoMessage(message,strlen(message)+1);

	int finished=0,sendtoPipe=0,totalSize_tobeSent=strlen(message)+1,numOfChars_toSent=bufferSize;
	int sentDatatoParent=1,written;
	while(sentDatatoParent){

		if(finished) break;
	
		if(!finished){
			if(sendtoPipe >= totalSize_tobeSent ){
				finished = True;
				continue;
			}
			char tempBuffer[numOfChars_toSent];
			strncpy(tempBuffer,message+sendtoPipe,numOfChars_toSent);
			written = write(fd,tempBuffer,numOfChars_toSent);
			switch(written){
				case -1: break;
				case 0:  break;
				default:
					printf("Worker: Write %d\n",written );
					sendtoPipe+=written;
					if((totalSize_tobeSent-sendtoPipe)<bufferSize)
						numOfChars_toSent = totalSize_tobeSent-sendtoPipe;
					break;
			}
		}	
	}
	close(fd);

	if(newfiles){
		for(int d=0;d<numOfDirs;d++){
			for(int f=0;f<numOfnewFiles[d];f++){
				if(statistics_toPipe[d][f]!=NULL)
					free(statistics_toPipe[d][f]);
			}
			if(statistics_toPipe[d]!=NULL)
				free(statistics_toPipe[d]);
		}
	}else{
		for(int d=0;d<numOfDirs;d++){
			for(int f=0;f<numOfFiles[d];f++){
				if(statistics_toPipe[d][f]!=NULL)
					free(statistics_toPipe[d][f]);
			}
			if(statistics_toPipe[d]!=NULL)
				free(statistics_toPipe[d]);
		}
	}
	free(statistics_toPipe);
	free(message);
	return 1;
}

void catchSIGUSR1(int signo){

	printf("Catching SIGUSR1\n");
	SIGUSR1_flag = 1;
}

void catchSIGINT_OR_SIGQUIT(int signo){

	printf("Catching SIGINT_OR_SIGQUIT\n");
	SIGINT_flag = 1;

}