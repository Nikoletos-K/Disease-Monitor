#include "patientData.h"

volatile sig_atomic_t SIGUSR1_flag=0;
volatile sig_atomic_t SIGINT_flag=0;
volatile sig_atomic_t SIGCHILD_flag=0;
volatile sig_atomic_t sigusr1_pid;

int main(int argc,char **argv){

	int index=1,b_index;
	int numOfWorkers=0,bufferSize=0;
	pid_t pid;
	DIR * input_dir;
	struct dirent * direntp;
	char cwd[PATH_MAX] = "./";

	static struct sigaction actSIGUSR1,actSIGINT_OR_SIGQUIT,actSIGCHILD;
	actSIGUSR1.sa_sigaction = catchSIGUSR1andPid;
	actSIGINT_OR_SIGQUIT.sa_handler = catchSIGINT_OR_SIGQUIT;
	actSIGCHILD.sa_handler = catchSIGCHILD;

	sigfillset(&(actSIGUSR1.sa_mask));
	sigfillset(&(actSIGINT_OR_SIGQUIT.sa_mask));
	sigfillset(&(actSIGCHILD.sa_mask));

	sigaction(SIGUSR1,&actSIGUSR1,NULL);
	sigaction(SIGINT,&actSIGINT_OR_SIGQUIT,NULL);
	sigaction(SIGQUIT,&actSIGINT_OR_SIGQUIT,NULL);
	sigaction(SIGCHLD,&actSIGCHILD,NULL);

	while(index<argc){
		/*- reading command-line arguments -*/
		switch(input(argv[index])){
			case w:
				numOfWorkers = atoi(argv[index+1]);
				break;
			case b:
				b_index = index+1;
				bufferSize = atoi(argv[index+1]);
				break;
			case i:
				input_dir = opendir(argv[index+1]);
				strcat(cwd,argv[index+1]);
				if(input_dir==NULL)
					printf("ERROR: Directory %s didn't open\n",argv[index+1]);
				break;
			case inputError:
				errorHandler(inputError,NULL);
				break;
		}
		index=index+2;
	}

	if(numOfWorkers<=0 || bufferSize<=0)	// Checking if arguments are valid
		errorHandler(commandLine,NULL);

	/* ------------ Reading and storing input directorys subdirs ----------*/
	int numOfSubDirs = 0;
	char **subdirsArray = malloc(sizeof(char *));
	while((direntp=readdir(input_dir))!=NULL){
		if(strcmp(direntp->d_name,".") && strcmp(direntp->d_name,"..")){
			char path[PATH_MAX];
			strcpy(path,cwd);
			strcat(path,"/");
			strcat(path,direntp->d_name);

			if(numOfSubDirs!=0)
				subdirsArray = realloc(subdirsArray,(numOfSubDirs+1)*sizeof(char*));
			subdirsArray[numOfSubDirs] = strdup(path);
			numOfSubDirs+=1;
		}
	}

	/* ------ Cheking if subdirs are less than workers ----------*/
	if(numOfWorkers>numOfSubDirs){
		numOfWorkers = numOfSubDirs;
		printf("Number of workers is geater than number of subdirectories,so %d Workers will be created \n",numOfWorkers);
	}

	npipe_connection **connectionsArray;
	connectionsArray = malloc(numOfWorkers*sizeof(npipe_connection *));

	/* ----- Creating worker processes ------ */
	for(int i=0;i<numOfWorkers;i++){

		connectionsArray[i] = create_pipeConnection(i);
		switch(pid = fork()){
			case -1:	
				/* if fork failed */
				perror("Fork failed");
				exit(1);
			case 0:		
				/* Child-Process (WORKER) */
				execlp("./worker","./worker","-pp",connectionsArray[i]->connection_names[PARENT],"-pc",connectionsArray[i]->connection_names[CHILD],"-b",argv[b_index],NULL);
				break;
			default:
				/* Parent-Process (SERVER) */
				connectionsArray[i]->pid = pid;
				break;
		}
	}

	/* ---- Preparing pipes for all the processes for passing the name of subdirs to workers -----*/
	
	int rc,wfd,rfd;
	struct pollfd fdarray[numOfWorkers*2];

	for(int i=0;i<2*numOfWorkers;i++)
		fdarray[i].fd = -1;

	int i=0;
	while(1){
		wfd = open(connectionsArray[i]->connection_names[PARENT],O_WRONLY);
		rfd = open(connectionsArray[i]->connection_names[CHILD],O_RDONLY|O_NONBLOCK);
		// printf("1. %s\n", connectionsArray[i]->connection_names[PARENT]);
		int j=0;
		while(j<numOfWorkers*2 && fdarray[j].fd>2) j++;
		if(j==numOfWorkers*2) break;

		if(wfd<0){
			if(errno==ENXIO)
				continue;
			else
				fprintf(stderr,"Pipe %s didn't open : %s \n",connectionsArray[i]->connection_names[PARENT],strerror(errno));
		}else{
			fdarray[i].fd = wfd;
			fdarray[i].events = POLLOUT;
			printf("w %d %d \n",i,wfd );

		}

		if(rfd<0){
			if(errno==ENXIO)
				continue;
			else
				fprintf(stderr,"Pipe %s didn't open : %s \n",connectionsArray[i]->connection_names[PARENT],strerror(errno));
		}else{
			fdarray[i+numOfWorkers].fd = rfd;
			fdarray[i+numOfWorkers].events = POLLIN;
			printf("r %d %d \n",i+numOfWorkers,rfd );
		}
		i++;		
		i=i%(numOfWorkers);
	}

	/* ---- Destributing subdirs to workers ----------*/

	int finished[numOfWorkers*2] ,sendtoPipe[numOfWorkers],totalSize_tobeSent[numOfWorkers],numOfChars_toSent[numOfWorkers];
	for(int i=0;i<numOfSubDirs;i++)
		insert_subdirectory(connectionsArray[i%numOfWorkers],subdirsArray[i]);
	

	int size;
	char ** Statistics = malloc(numOfWorkers*sizeof(char *));
	int totalSize_tobeRead[numOfWorkers];
	int sizeofReads[numOfWorkers],sizeindex[numOfWorkers];
	char **sizebuffer = malloc(sizeof(char*)*numOfWorkers);

	for(int i=0;i<numOfWorkers;i++){
		totalSize_tobeRead[i] = -1;
		sizeofReads[i] = 0;
		sizeindex[i] = 0;
		sizebuffer[i] = malloc(sizeof(char)*50);
		Statistics[i] = malloc(sizeof(char));
		strncpy(Statistics[i],"",1);
		size = strlen(connectionsArray[i]->subdirs_concat)+1;
		connectionsArray[i]->subdirs_concat = insertSizetoMessage(connectionsArray[i]->subdirs_concat,size);
		finished[i] = False;
		sendtoPipe[i] = 0;
		totalSize_tobeSent[i] = strlen(connectionsArray[i]->subdirs_concat)+1;
		numOfChars_toSent[i] = bufferSize;
	}

	for(int i=0;i<numOfWorkers*2;i++)
		finished[i] = False;



	/* ---- Passing the name of subdirs to workers ------- */
	int written,pipe_index;
	int TransmitingData=1;
	while(TransmitingData){

		int j=0;
		while(j<numOfWorkers*2 && finished[j]) j++;
		if(j==numOfWorkers*2)	break;

		rc = poll(fdarray,numOfWorkers*2,-1);

		if(rc==0){
			perror("Server: poll timed-out ");
			break;
		}else if(rc>0){
		
			for(int f=0;f<numOfWorkers*2;f++){	
				pipe_index = f;

				if(pipe_index<numOfWorkers && (fdarray[pipe_index].revents & POLLOUT) && finished[f]!=True) {
					
					if(sendtoPipe[f] >= totalSize_tobeSent[f] ){
						finished[f] = True;
						// close(fdarray[pipe_index].fd);
						// fdarray[pipe_index].fd*=-1;
						continue;
					}
					char tempBuffer[numOfChars_toSent[f]];
					strncpy(tempBuffer,connectionsArray[f]->subdirs_concat+sendtoPipe[f],numOfChars_toSent[f]);
					written = write(fdarray[pipe_index].fd,tempBuffer,numOfChars_toSent[f]);
					sendtoPipe[f]+=written;
					if((totalSize_tobeSent[f]-sendtoPipe[f])<bufferSize)
						numOfChars_toSent[f] = totalSize_tobeSent[f]-sendtoPipe[f];
				
				}

				if(pipe_index>=numOfWorkers && (fdarray[pipe_index].revents & POLLIN) && finished[f]!=True) {

					printf("Server: Smth to read\n");
					int read_index = f-numOfWorkers;
					char buffer[bufferSize];
					int bytesRead=read(fdarray[pipe_index].fd,buffer,bufferSize);

					switch(bytesRead){
						case -1:
							if(errno == EAGAIN)
								break;
							if(errno == EINTR)
								printf("STOP\n");
							break;
						case 0:
							break;
						default:
							printf("Server: READ %d \n",bytesRead);
							Statistics[read_index] = realloc(Statistics[read_index],sizeofReads[read_index]+bytesRead);
							strncpy(Statistics[read_index]+sizeofReads[read_index],buffer,bytesRead);
							sizeofReads[read_index] += bytesRead;
							break;
					}
			
					if(totalSize_tobeRead[read_index] != -1 && sizeofReads[read_index]>=totalSize_tobeRead[read_index]+sizeindex[read_index]+1){
						printf("Server: end-read %s\n", Statistics[read_index]);
						finished[f]=True;
						continue;
					}
							
					if(totalSize_tobeRead[read_index]==-1 && bytesRead>0){

						// printf("read_index %d\n",read_index );
						while(sizeindex[read_index]<sizeofReads[read_index] && Statistics[read_index][sizeindex[read_index]] !='%' ){
							sizebuffer[read_index][sizeindex[read_index]] = Statistics[read_index][sizeindex[read_index]];
							sizeindex[read_index]++;
						}

						// printf("sizeindex[read_index] %d\n",read_index );
						if(sizeindex[read_index]<sizeofReads[read_index] && Statistics[read_index][sizeindex[read_index]]=='%'){
							sizebuffer[read_index][sizeindex[read_index]] = '\0';
							totalSize_tobeRead[read_index] = atoi(sizebuffer[read_index]);
							printf("++ %d\n",totalSize_tobeRead[read_index]+sizeindex[read_index]+1);

							if(sizeofReads[read_index]>=totalSize_tobeRead[read_index]+sizeindex[read_index]+1){
								printf("Server: end-read %s\n", Statistics[read_index]);
								finished[f]=True;
								continue;
							}
						}

					}
				}
			}
		}else if(rc==-1){
			printf("%s\n",strerror(errno) );
			exit(1);
		}
	}

	/* ----- Printing summary Statistics from worker processes  ------ */

	printStatistics(Statistics,numOfWorkers);

	/* ----- Reading and distributing subdirectories to worker processes  ------ */
	// int flag = fcntl(0,F_GETFD);
	// fcntl(0,F_SETFD,flag|O_NONBLOCK);


	int stayAlive=1;
	fflush(stdout);

	struct pollfd input[1];
	input[0].fd = 0;
	input[0].events = POLLIN;

	printf("> ");
	fflush(stdout);			
	while(stayAlive){

		int pp = poll(input,1,1);

		if(pp==0){
			if(SIGUSR1_flag){
				printf("New Statistics: \n");
				ReadAndPrintSummaryStatistics(numOfWorkers,fdarray+numOfWorkers,bufferSize,sigusr1_pid,connectionsArray);
				SIGUSR1_flag=0;
				printf("\n> ");			
				fflush(stdout);			
			}

			if(SIGINT_flag){
				stayAlive=0;
				for(int i=0;i<numOfWorkers;i++)
					kill(SIGKILL,connectionsArray[i]->pid);
			}

			if(SIGCHILD_flag){

				pid_t temppid,deadpid;
				int status;
				while((temppid=waitpid(-1,&status,WNOHANG)) != -1)
					deadpid = temppid;

				printf("%ld\n",(long)deadpid);
				replaceChild(deadpid,numOfWorkers,connectionsArray,argv[b_index]);
				SIGCHILD_flag=0;
				printf("\n> ");			
				fflush(stdout);			

			}
		}else if(pp==1 && (input[0].revents & POLLIN)){
			char line[100];
			fscanf(stdin,"%s",line);

			// char date1[DATE_BUFFER],date2[DATE_BUFFER];
			switch(Prompt(line))
			{
				case LIST:
					listCountries(numOfWorkers,connectionsArray);
					break;
				case DISEASE_FREQUENCY:
					break;				
				case TOPK_AGE_RANGES:
					break;
				case SEARCH_PATIENT:
					break;
				case NUM_PDISCHARGES:
					break;
				case NUM_PADMISSIONS:
					break;
				case EXIT:
					printf("DiseaseAggregator ends\n");
					stayAlive=False;
					break;				
				case INPUT_ERROR:
					printf("\n\tERROR: Command not found\n\n");
					break;
				case NO_INPUT:
					break;
				default:
					break;

			}
			if(stayAlive){
				printf("\n> ");			
				fflush(stdout);			
			}
		}
	}	
	for(int i=0;i<numOfWorkers;i++)
		kill(connectionsArray[i]->pid,SIGINT);

	while(wait(NULL)>0);

	closedir(input_dir);
	for(int i=0;i<numOfWorkers;i++){
		free(sizebuffer[i]);
		free(Statistics[i]);
		destroy_pipeConnection(connectionsArray[i]);
	}

	for(int i=0;i<numOfSubDirs;i++){
		free(subdirsArray[i]);
	}
	free(sizebuffer);
	free(Statistics);
	free(connectionsArray);
	free(subdirsArray);
return 0;
}

void catchSIGUSR1andPid(int signo,siginfo_t *info,void *data){

	printf("Parent: Catching SIGUSR1\n");
	sigusr1_pid = info->si_pid;
	SIGUSR1_flag = 1;
}

void catchSIGINT_OR_SIGQUIT(int signo){

	printf("Parent: Catching SIGINT_OR_SIGQUIT\n");
	SIGINT_flag = 1;

} 

void catchSIGCHILD(int signo){

	printf("Parent: SIGCHILD\n");
	SIGCHILD_flag = 1;
} 