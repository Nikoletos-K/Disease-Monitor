#include "patientData.h"

RBTNode ** inodeRBTree;
RBTNode ** RBTreeArray;
char ***filesArray;
char * pipename[2];
int bufferSize;
char **dirsArray;
int numOfDirs;
HashTable ** countryHashTable;
HashTable * diseaseHashTable;
RBTNode * idRBT;
volatile sig_atomic_t SIGUSR1_flag=0;
volatile sig_atomic_t SIGINT_flag=0;
int SUCCESS;
int FAIL;


int main(int argc,char **argv){

	fprintf(stdout,"%ld: Worker created \n",(long)getpid());
	/* -----  Signal sets,masks and handlers initialazation  ----- */
	sigset_t blocksignals;
	sigfillset(&blocksignals);
	sigprocmask(SIG_SETMASK,&blocksignals,NULL);
	
	int i=1;
	
	char size[50];
	int bytesRead,sizeofReads=0,readfd=-1,writefd=-1,index=0,packageSize = -1;
	initializeDataStructures();
	diseaseHashTable = create_HashTable(100,516);
	idRBT = RBTConstruct();

	static struct sigaction actSIGUSR1,actSIGINT;
	actSIGUSR1.sa_handler = catchSIGUSR1;
	actSIGINT.sa_handler = catchSIGINT_OR_SIGQUIT;

	sigfillset(&(actSIGUSR1.sa_mask));
	sigfillset(&(actSIGINT.sa_mask));

	sigaction(SIGUSR1,&actSIGUSR1,NULL);
	sigaction(SIGINT,&actSIGINT,NULL);
	sigaction(SIGQUIT,&actSIGINT,NULL);

	/* ----- Reading command-line arguments ----- */
	while(argv[i]!=NULL){
		if(!strcmp(argv[i],"-pc"))
			pipename[CHILD] = strdup(argv[i+1]);
		else if(!strcmp(argv[i],"-pp"))
			pipename[PARENT] = strdup(argv[i+1]);
		else if(!strcmp(argv[i],"-b"))
			bufferSize = atoi(argv[i+1]);
		i++;			
	}

	while((readfd=open(pipename[PARENT],O_RDONLY))<0);	// Opening pipe for reading subdirectories

	/* --------------- Reading from pipe directories ------------------- */ 
	// printf("(%ld) ^^ fd %d\n",(long)getpid(),readfd );
	char * pipeRead = readfromPipe(bufferSize,readfd);
	fprintf(stdout,"%ld: Worker read directories from master \n",(long)getpid());
	// printf("+++  %s %ld\n",pipeRead,(long)getpid() );

	/* --------------- Reading from pipe server IP and Port ------------------- */ 

	char * IPandPort = readfromPipe(bufferSize,readfd);
	// printf("-- %s %ld\n",IPandPort,(long)getpid() );
	char IP[50],PORT[50];
	IP[0]='\0',PORT[0]='\0';
	int c=0;
	char * token = strtok(IPandPort,"%");
	while(token!=NULL){
		if(c==1)
			strcpy(PORT,token);
		else if(c==2)
			strcpy(IP,token);

		token = strtok(NULL,"%");
		c++;
	}
	fprintf(stdout,"%ld: Worker read IP and Port from master \n",(long)getpid());

	/* ------------ Opening subdirs and reading records for each subdir ------------- */
	dirsArray = malloc(sizeof(char*));

	/* ------ Split strings to eash subdir --------- */

	token = strtok(pipeRead,"%");
	while(token!=NULL){
		token = strtok(NULL,"%");
		if(token == NULL) break;
		dirsArray = realloc(dirsArray,(numOfDirs+1)*sizeof(char*));
		dirsArray[numOfDirs] = strdup(token);
		numOfDirs++;
	}

	countryHashTable = malloc(numOfDirs*sizeof(HashTable *));
	for(int i=0;i<numOfDirs;i++)
		countryHashTable[i] = create_HashTable(100,516);
	
	
	filesArray = malloc(numOfDirs*sizeof(char**)); 
	RBTreeArray = malloc(sizeof(RBTNode*)*numOfDirs);
	int * numOfFiles = malloc(numOfDirs*sizeof(int));
	inodeRBTree = malloc(sizeof(RBTNode*)*numOfDirs);
	for(int d=0;d<numOfDirs;d++){
		RBTreeArray[d] = RBTConstruct();
		inodeRBTree[d] = RBTConstruct();
	}

	/* ----- Creating socket ------- */
	struct hostent * foundhost;
	struct in_addr address;
	int serverSocket,workerSocket;
	struct sockaddr_in server;
	struct sockaddr * serveptr = (struct sockaddr *)&server;

	/* Establishing connection with Server */
	int port = atoi(PORT);
	
	if((serverSocket = socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	while(inet_pton(AF_INET,IP,&address)<=0){
		if(errno==EAGAIN)
			continue;
		perror("inet_pton");
		exit(EXIT_FAILURE);
	}

	if((foundhost = gethostbyaddr((const char *)&address,sizeof(address),AF_INET))==NULL){
		herror("gethostbyaddr");
		exit(EXIT_FAILURE);
	}

	server.sin_family = AF_INET;
	memcpy(&(server.sin_addr),foundhost->h_addr,foundhost->h_length);
	server.sin_port = htons(port);

	while(connect(serverSocket,serveptr,sizeof(server))){
		if(errno == ECONNREFUSED)
			continue;
		perror("Worker socket connection failed");
		exit(EXIT_FAILURE);
	}
	
	/* ------ Creating workers socket ------ */

	struct sockaddr_in client,queries_fromServer;
	socklen_t serverlen = sizeof(client);
	struct sockaddr * clientptr = (struct sockaddr *) &client;
	struct sockaddr * serverptr = (struct sockaddr *) &queries_fromServer;

	int newSocket;
	if((workerSocket = socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("Socket creation failed");fflush(stdout);
		exit(EXIT_FAILURE);
	}

	client.sin_family = AF_INET;
	client.sin_addr.s_addr = htonl(INADDR_ANY);
	client.sin_port = 0;

	if(bind(workerSocket,clientptr,sizeof(client))<0){
		perror("Server socket bind failed (statistics)");fflush(stdout);
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in tempserver;
	struct sockaddr  * tempaddr = (struct sockaddr  *)&tempserver;
	socklen_t templen = sizeof(tempserver);

	if(getsockname(workerSocket,tempaddr,&templen)==-1){
		perror("getsockname");fflush(stdout);
		exit(EXIT_FAILURE);
	}

	int newPort = ntohs(tempserver.sin_port);


	/* ----- Sending new port ----- */

	write(serverSocket,&newPort,sizeof(int));

	/* ---- Sending statistics ---- */

	CalculateAndSendStatistics(INITIAL_FILES,numOfFiles,serverSocket);

	fprintf(stdout,"%ld: Worker send Port and Statistics to whoClient\n",(long)getpid());

	/* ------ Getting ready for queries in the new port ---- */

	if(listen(workerSocket,SOMAXCONN)<0){
		perror("Listen to statistics sockets failed");fflush(stdout);
		exit(EXIT_FAILURE);
	}

	sigprocmask(SIG_UNBLOCK,&blocksignals,NULL);

	/* --------- Communication with diseaseAggregator ---------------------*/
	struct pollfd fdarray[1];
	fdarray[0].fd = workerSocket;
	fdarray[0].events = POLLIN|POLLOUT;
	int event;

	char * command = malloc(sizeof(char));
	size[0] = '\0';
	bytesRead=0,sizeofReads=0,index=0,packageSize=-1;
	int sendtoPipe=0,totalSize_tobeSent,numOfChars_toSent=bufferSize,written;
	char * results = NULL;
	int workerRun = 1,blockonce=1;
	int finish=0;
	fprintf(stdout,"%ld: Worker ready for requests from whoClient\n",(long)getpid());

	while(workerRun){

		event = poll(fdarray,1,-1);		// poll waits for action in filedesc to read or write

		if(SIGUSR1_flag){

			sigprocmask(SIG_SETMASK,&blocksignals,NULL);	// block 
			fprintf(stdout,"%ld: Catching SIGUSR1\n",(long)getpid());
			if(CalculateAndSendStatistics(NEW_FILES,numOfFiles,serverSocket)){
				kill(getppid(),SIGUSR1);
			}else{
				printf("No new files to any directory\n");
				fflush(stdout);
			}

			SIGUSR1_flag = 0;

			sigprocmask(SIG_UNBLOCK,&blocksignals,NULL);	// unblock

		}

		if(SIGINT_flag){

			fprintf(stdout,"%ld: Catching SIGINT\n",(long)getpid());
			char filename[50];
			sprintf(filename,"log_file.%ld",(long)getpid());
			FILE * log = fopen(filename,"w+");
			for(int i=0;i<numOfDirs;i++){
				char country[50];
				getCountry(dirsArray[i],country);
				fprintf(log,"%s\n",country);fflush(log);
			}
			fprintf(log,"TOTAL %d\n",FAIL+SUCCESS);fflush(log);
			fprintf(log,"SUCCESS %d\n",SUCCESS);fflush(log);
			fprintf(log,"FAIL %d\n",FAIL);fflush(log);				
			workerRun=0;
			fclose(log);
		}
		
		if(event>0){	// Query came

			if((newSocket=accept(workerSocket,serverptr,&serverlen))<0){
				perror("Accept statistics socket");
				exit(EXIT_FAILURE);
			}else
				fprintf(stdout,"%ld: Request from whoClient\n",(long)getpid());


			while(1){ // loop untill all data has been send to socket
			
				if((fdarray[0].revents & POLLIN)){		// smth to read

					if(blockonce){
						sigprocmask(SIG_SETMASK,&blocksignals,NULL);
						blockonce=0;
					}

					char buffer[bufferSize];
					bytesRead=read(newSocket,buffer,bufferSize);
					switch(bytesRead){
						case -1:
							if(errno == EAGAIN)
								break;
							if(errno == EINTR)
								printf("STOP\n");
							break;
						case 0:	break;
						default:
							command = realloc(command,(sizeofReads+bytesRead)*sizeof(char));
							strncpy(command+sizeofReads,buffer,bytesRead);
							sizeofReads += bytesRead;
							break;
					}

					if(packageSize != -1 && sizeofReads>=packageSize+index+1){		// if read whole command execute it
						results = malloc(sizeof(char));
						results[0] = '\0';
						results = executeCommand(command,results);
						results = insertSizetoMessage(results,strlen(results)+1);
						totalSize_tobeSent = strlen(results)+1;
						size[0] = '\0';
						finish=0;
						bytesRead=0,sizeofReads=0,index=0,packageSize=-1;
						free(command);
						command = malloc(sizeof(char));
					}
							
					if(packageSize==-1 && bytesRead>0){

						while(index<sizeofReads && command[index]!='%' ){
							size[index] = command[index];
							index++;
						}
						if(index<sizeofReads && command[index]=='%'){
							size[index] = '\0';
							packageSize = atoi(size);

							if(sizeofReads>=packageSize+index+1){	// if read whole command execute it
								results = malloc(sizeof(char));
								results[0] = '\0';
								results = executeCommand(command,results);
								results = insertSizetoMessage(results,strlen(results)+1);
								totalSize_tobeSent = strlen(results)+1;
								size[0] = '\0';
								finish=0;
								bytesRead=0,sizeofReads=0,index=0,packageSize=-1;
								free(command);
								command = malloc(sizeof(char));
							}
						}
					}
				}

				if(results!=NULL){	// if theres smth to write and has the right

					while(1){
						if(sendtoPipe >= totalSize_tobeSent ){	// if finished writing prepare for new command
							fflush(stdout);
							sendtoPipe=0,totalSize_tobeSent=-1;
							free(results);
							results = NULL;
							numOfChars_toSent = bufferSize;
							sigprocmask(SIG_UNBLOCK,&blocksignals,NULL);	// unblock
							blockonce=1;
							finish=1;
							break;
						}else{
							char tempBuffer[numOfChars_toSent];tempBuffer[0] = '\0';
							strncpy(tempBuffer,results+sendtoPipe,numOfChars_toSent);
							written = write(newSocket,tempBuffer,numOfChars_toSent);
							switch(written){
								case -1: break;
								case 0:  break;
								default:
									sendtoPipe+=written;
									if((totalSize_tobeSent-sendtoPipe)<bufferSize)
										numOfChars_toSent = totalSize_tobeSent-sendtoPipe;
									break;
							}
						}
					}
					if(finish){
						fprintf(stdout,"%ld: Request served\n",(long)getpid());
						fflush(stdout);
						break;
					}
				}
			}
			close(newSocket);		
		}else if(event<0){
			perror("Worker poll failed");
			exit(1);
		}
	}
	/*----  Closing server socket ---- */
	close(serverSocket);

	/* ----  Free of allocated memory ----- */
	for(int d=0;d<numOfDirs;d++){
		for(int f=0;f<numOfFiles[d];f++){
			free(filesArray[d][f]);
		}
		free(dirsArray[d]);
		free(filesArray[d]);
		RBTDestroyTreeAndData(RBTreeArray[d],destroyPatient);
		RBTDestroyAllNodes(inodeRBTree[d]);
		destroyHashTable(countryHashTable[d]);
	}
	free(command);
	destroyHashTable(diseaseHashTable);
	free(countryHashTable);
	RBTDestroyAllNodes(idRBT);		
	free(filesArray);
	free(dirsArray);
	free(RBTreeArray);
	free(inodeRBTree);
	free(numOfFiles);
	free(pipeRead);
	free(pipename[PARENT]);
	free(pipename[CHILD]);
	destroyDataStructures();
	close(writefd);
	close(readfd);
	printf("->Worker %ld ended \n",(long)getpid());
	return 0;
}

int CalculateAndSendStatistics(int newfiles,int *numOfFiles,int writefd){

	DIR * input_dir;
	struct dirent * direntp;
	int numOfnewFiles[numOfDirs],f;
	char ***statistics_toPipe = malloc(numOfDirs*sizeof(char**));

	for(int d=0;d<numOfDirs;d++){	// for every directory that worker handles
		input_dir = opendir(dirsArray[d]);
		if(input_dir==NULL)
			fprintf(stderr,"Worker %ld : opendir %s\n",(long)getpid(),strerror(errno));
		
		numOfnewFiles[d]=0;
		RBTNode * tempTree;
		if(newfiles){	// case that SIGUSR1 with new files
			tempTree = RBTConstruct();
			for(int f=0;f<numOfFiles[d];f++)				
				RBTInsert(&tempTree,(void*)filesArray[d][f],(void*)filesArray[d][f],stringComparator);
			f=numOfFiles[d];
		}else{		// case of first initialazation
			f=0;
			filesArray[d] = malloc(sizeof(char*));
		}
	

		while((direntp=readdir(input_dir))!=NULL){		// opening and reading all files from directory
			if(strcmp(direntp->d_name,".") && strcmp(direntp->d_name,"..")){
				if(RBTFindNode(inodeRBTree[d],(void *)direntp->d_name,stringComparator)==NULL){ // if file not exists read 
					filesArray[d] = realloc(filesArray[d],(f+1)*sizeof(char*));
					filesArray[d][f] = strdup(direntp->d_name);
					RBTInsert(&inodeRBTree[d],(void *)direntp->d_name,(void *)direntp->d_name,stringComparator);
					f++;
					if(newfiles)
						numOfnewFiles[d]++;
				}
			}
		}
		if(newfiles)
			numOfFiles[d] += numOfnewFiles[d];
		else
			numOfFiles[d] = f;


		if(newfiles && numOfnewFiles[d]==0){	// if no new files
			statistics_toPipe[d]=NULL;
			RBTDestroyAllNodes(tempTree);
			closedir(input_dir);
			continue;
		}else{
			qsort(filesArray[d],numOfFiles[d],sizeof(char*),dateCompare);	// sort them as dates
			if(newfiles)
				statistics_toPipe[d] = malloc(sizeof(char*)*numOfnewFiles[d]);
			else
				statistics_toPipe[d] = malloc(sizeof(char*)*numOfFiles[d]);
		}

		int statistics_index=0;
		for(int i=0;i<numOfFiles[d];i++){	// for every file in directory

			if(newfiles && RBTFindNode(tempTree,(void *)filesArray[d][i],stringComparator)!=NULL){
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

			HashTable * diseaseHT_perDay = create_HashTable(100,516);
			
			statistics_toPipe[d][statistics_index] = malloc((strlen(country)+strlen(filesArray[d][i])+3)*sizeof(char));
			strcpy(statistics_toPipe[d][statistics_index],"");
			strcat(statistics_toPipe[d][statistics_index],filesArray[d][i]);
			strcat(statistics_toPipe[d][statistics_index],"%");
			strcat(statistics_toPipe[d][statistics_index],country);
			strcat(statistics_toPipe[d][statistics_index],"%");
			
			while(!feof(tempfile)){		// read all lines
				char id[100],fname[100],lname[100],enterORexit[100],disease[100];int age;
				int elems = fscanf(tempfile,"%s %s %s %s %s %d \n",id,enterORexit,fname,lname,disease,&age);
				if(elems < 6 || !onlyLettersandNumbers(id) || !(!strcmp(enterORexit,"ENTER") || !strcmp(enterORexit,"EXIT"))){
					fprintf(stderr,"ERROR\n");
					continue;
				}
				if(!strcmp(enterORexit,"ENTER")){	// case ENTER
					if(RBTFindNode(RBTreeArray[d],(void*)id,stringComparator)==NULL){		// if this patient not exist in system
						Patient * p = createPatient(id,fname,lname,disease,age,filesArray[d][i],country);
						RBTInsert(&RBTreeArray[d],(void *) p,p->id,stringComparator);
						insert_toHashTable(p,p->disease,p->id,diseaseHT_perDay,stringComparator);
						insert_toHashTable(p,p->disease,p->entryDate,diseaseHashTable,dateComparator);
						insert_toHashTable(p,p->disease,p->entryDate,countryHashTable[d],dateComparator);
						RBTInsert(&idRBT,(void *)p,p->id,stringComparator);
					}
					else 	// if patient exists and has enter -> ERROR
						fprintf(stderr,"ERROR\n");
				}else{	// if patient exists and has EXIT
					RBTNode * temp;
					if((temp=RBTFindNode(RBTreeArray[d],(void*)id,stringComparator))!=NULL){
						Patient * p = (Patient*) get_RBTData(temp);
						if(dateCompare(&(p->entryDate),&(filesArray[d][i]))<=0)
							infromExitDate(p,filesArray[d][i]);
						else 	// if date of entry is later than exit -> ERRROR
							fprintf(stderr,"ERROR\n");
					}else // if patient ENTER not in system -> ERRROR
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

	/* create one message to be passed through a pipe */
	char * message;
	if(newfiles)
		message = concat_toMessage(statistics_toPipe,numOfDirs,numOfnewFiles);		
	else
		message = concat_toMessage(statistics_toPipe,numOfDirs,numOfFiles);

	message = insertSizetoMessage(message,strlen(message)+1);

	int finished=0,sendtoPipe=0,totalSize_tobeSent=strlen(message)+1,numOfChars_toSent=bufferSize;
	int sentDatatoParent=1,written;

printf("%ld writing to sock \n",(long)getpid());
	/* write to the pipe */
	while(sentDatatoParent){

		if(finished) break;
	
		if(!finished){
			if(sendtoPipe >= totalSize_tobeSent ){
				finished = True;
				continue;
			}
			char tempBuffer[numOfChars_toSent];
			strncpy(tempBuffer,message+sendtoPipe,numOfChars_toSent);
			written = write(writefd,tempBuffer,numOfChars_toSent);
			switch(written){
				case -1: break;
				case 0:  break;
				default:
					sendtoPipe+=written;
					if((totalSize_tobeSent-sendtoPipe)<bufferSize)
						numOfChars_toSent = totalSize_tobeSent-sendtoPipe;
					break;
			}
		}	
	}

	/* Free all allocated memory */
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


char * executeCommand(char * command,char * result){
	/* Function that splits and executes command derived from parent */

	char * arguments[7];
	for(int i=0;i<7;i++) arguments[i] = NULL;
	int numOfArguments=0;
	char * token = strtok(command,"%");
	while(token!=NULL){
		arguments[numOfArguments] = strdup(token);
		numOfArguments++;
		token = strtok(NULL,"%");
	}

	int cmd;
	switch(cmd=Prompt(arguments[1])){

		case DISEASE_FREQUENCY:
		{
			int numOfPatients,i=0;
			if(arguments[5]!=NULL){
				int found=0;
				for(i=0;i<numOfDirs;i++){
					char country[100],temp[100];
					temp[0] = '\0';
					strcat(temp,dirsArray[i]);
					getCountry(temp,country);
					if(!strcmp(country,arguments[5])){
						found=1;
						break;
					}
				}
				if(!found)
					numOfPatients=0;
				else
					numOfPatients = diseaseFrequency(arguments[2],arguments[3],arguments[4],arguments[5],countryHashTable[i]);				
			}
			else
				numOfPatients = diseaseFrequency(arguments[2],arguments[3],arguments[4],arguments[5],diseaseHashTable);

			if(numOfPatients<0)
				numOfPatients=0;
			char buffer[50];buffer[0] = '\0';
			sprintf(buffer,"%d",numOfPatients);
			result = realloc(result,strlen(buffer)+1);
			strcpy(result,buffer);
			SUCCESS++;
			break;
		}
		case TOPK_AGE_RANGES:
		{	
			int h=0;
			int found=0;
			for(h=0;h<numOfDirs;h++){
				char country[100],temp[100];
				temp[0] = '\0';
				strcat(temp,dirsArray[h]);
				getCountry(temp,country);
				if(!strcmp(country,arguments[3])){
					found=1;
					break;
				}
			}
			if(!found){
				char buffer[200];
				strcpy(buffer,"-");
				result = realloc(result,strlen(buffer)+1);
				strcpy(result,buffer);
			}else{
				result = topk(atoi(arguments[2]),arguments[4],arguments[5],arguments[6],countryHashTable[h],result);
				SUCCESS++;
			}
			break;
		}
		case SEARCH_PATIENT:
		{
			Patient * p;
			RBTNode * temp;			
			char buffer[200];
			if((temp = RBTFindNode(idRBT,(void*)arguments[2],stringComparator))!=NULL){
				p=(Patient *)get_RBTData(temp);
				sprintf(buffer,"%s %s %s %s %d %s %s",p->id,p->firstName,p->lastName,p->disease,p->age,p->entryDate,p->exitDate);
			}else
				strcpy(buffer,"-");
			result = realloc(result,strlen(buffer)+1);
			strcpy(result,buffer);
			SUCCESS++;
			break;
		}
		case NUM_PADMISSIONS:
		case NUM_PDISCHARGES:
		{	
			int found=0;
			if(arguments[5]!=NULL){
				for(int i=0;i<numOfDirs;i++){
					char country[100],temp[100];
					temp[0] = '\0';
					strcat(temp,dirsArray[i]);
					getCountry(temp,country);
					if(!strcmp(country,arguments[5])){
						found=1;
						break;
					}
				}

				if(!found){
					char buffer[100];buffer[0]= '\0';
					strcpy(buffer,"-");
					result = realloc(result,strlen(buffer)+1);
					strcpy(result,buffer);
					break;
				}
			}

			char * buffer = malloc(sizeof(char));
			buffer[0] = '\0';
			int numOfPatients;
			for(int i=0;i<numOfDirs;i++){
				char country[100],temp[100];
				temp[0] = '\0';
				strcat(temp,dirsArray[i]);
				getCountry(temp,country);
				if(arguments[5]!=NULL && !strcmp(country,arguments[5])){

					if(cmd==NUM_PADMISSIONS)
						numOfPatients = diseaseFrequency(arguments[2],arguments[3],arguments[4],arguments[5],countryHashTable[i]);
					else
						numOfPatients = discharges(arguments[2],arguments[3],arguments[4],arguments[5],countryHashTable[i]);

					if(numOfPatients<0)
						numOfPatients=0;

					char tempBuffer[100];
					sprintf(tempBuffer,"%s %d",country,numOfPatients);
					buffer = realloc(buffer,strlen(tempBuffer)+1);
					strcpy(buffer,tempBuffer);
					break;
				}else{
					if(cmd==NUM_PADMISSIONS)
						numOfPatients = diseaseFrequency(arguments[2],arguments[3],arguments[4],arguments[5],countryHashTable[i]);
					else
						numOfPatients = discharges(arguments[2],arguments[3],arguments[4],arguments[5],countryHashTable[i]);
	
					if(numOfPatients<0)
						numOfPatients=0;

					char tempBuffer[100];tempBuffer[0]='\0';
					sprintf(tempBuffer,"%s %d",country,numOfPatients);
					buffer = realloc(buffer,strlen(buffer)+strlen(tempBuffer)+2);
					strcat(buffer,tempBuffer);
					strcat(buffer,"%");
				}
			}

			if(numOfPatients<0)
				numOfPatients=0;
			result = realloc(result,strlen(buffer)+1);
			strcpy(result,buffer);
			free(buffer);
			SUCCESS++;
			break;
		}
		default:
			break;
	}

	for(int i=0;i<numOfArguments;i++)
		free(arguments[i]);

	return result;
}

void catchSIGUSR1(int signo){
	SIGUSR1_flag = 1;
}

void catchSIGINT_OR_SIGQUIT(int signo){
	SIGINT_flag = 1;
}