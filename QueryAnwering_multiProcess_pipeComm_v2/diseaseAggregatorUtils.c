#include "patientData.h"

void listCountries(int numOfWorkers,npipe_connection **connectionsArray){

	for(int i=0;i<numOfWorkers;i++){
		for(int d=0;d<connectionsArray[i]->numOfSubdirs;d++){
			char country[50],buffer[50];buffer[0] = '\0';
			strcat(buffer,connectionsArray[i]->subdirectories[d]);
			getCountry(buffer,country);
			printf("%s -> Process id: %ld \n",country,(long)connectionsArray[i]->pid);
		}
	}
}

void ReadAndPrintSummaryStatistics(int numOfWorkers,struct pollfd * fdarray,int bufferSize,pid_t senderpid,npipe_connection **connectionsArray){

	// struct pollfd fdarray[numOfWorkers];
	// for(int i=0;i<numOfWorkers;i++)
	// 	fdarray[i] = ffdarray[i+numOfWorkers];

	int totalSize_tobeRead[numOfWorkers];
	int sizeofReads[numOfWorkers],sizeindex[numOfWorkers];
	char **sizebuffer = malloc(sizeof(char*)*numOfWorkers);
	char ** Statistics = malloc(numOfWorkers*sizeof(char *));
	int finished[numOfWorkers];
	
	for(int i=0;i<numOfWorkers;i++){
		Statistics[i] = malloc(sizeof(char));
		strncpy(Statistics[i],"",1);
		if(connectionsArray[i]->pid==senderpid)
			finished[i] = False;
		else
			finished[i] = True;
		totalSize_tobeRead[i] = -1;
		sizeofReads[i] = 0;
		sizeindex[i] = 0;
		sizebuffer[i] = malloc(sizeof(char)*50);
	}

	int TransmitingData=1,rc;
	while(TransmitingData){

		int j=0;
		while(j<numOfWorkers && finished[j]) j++;
		if(j==numOfWorkers)	break;

		rc = poll(fdarray,numOfWorkers,-1);
		
		if(rc==0){
			perror("Server: poll timed-out ");
			break;
		}else if(rc>0){
		
			for(int f=0;f<numOfWorkers;f++){	
				if((fdarray[f].revents & POLLIN) && finished[f]!=True) {

					char buffer[bufferSize];
					int bytesRead=read(fdarray[f].fd,buffer,bufferSize);
					printf("READ %d\n",bytesRead);
					printf("numOfWorkers %d\n",numOfWorkers);
					switch(bytesRead){
						case -1:
							if(errno == EAGAIN)
								break;
							if(errno == EINTR)
								printf("STOP\n");
							break;
						case 0:	break;
						default:
							Statistics[f] = realloc(Statistics[f],sizeofReads[f]+bytesRead);
							strncpy(Statistics[f]+sizeofReads[f],buffer,bytesRead);
							sizeofReads[f] += bytesRead;
							break;
					}
			
					if(totalSize_tobeRead[f] != -1 && sizeofReads[f]>=totalSize_tobeRead[f]+sizeindex[f]+1){
						finished[f]=True;
						printf("FINITO\n");
						continue;
					}
							
					if(totalSize_tobeRead[f]==-1 && bytesRead>0){

						while(sizeindex[f]<sizeofReads[f] && Statistics[f][sizeindex[f]] !='%' ){
							sizebuffer[f][sizeindex[f]] = Statistics[f][sizeindex[f]];
							sizeindex[f]++;
						}

						if(sizeindex[f]<sizeofReads[f] && Statistics[f][sizeindex[f]]=='%'){
							sizebuffer[f][sizeindex[f]] = '\0';
							totalSize_tobeRead[f] = atoi(sizebuffer[f]);

							if(sizeofReads[f]>=totalSize_tobeRead[f]+sizeindex[f]+1){
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
	printf("Ready to print\n");
	printStatistics(Statistics,numOfWorkers);

	for(int i=0;i<1;i++){
		free(sizebuffer[i]);
		free(Statistics[i]);
	}

	free(sizebuffer);
	free(Statistics);
}

void replaceChild(pid_t deadpid,int numOfWorkers,npipe_connection **connectionsArray,char * chbufferSize){

	printf("%ld\n",(long)deadpid);
	pid_t pid;
	int n,bufferSize=atoi(chbufferSize);
	for(n=0;n<numOfWorkers;n++){
		printf("%ld\n",(long)connectionsArray[n]->pid);
		if(connectionsArray[n]->pid == deadpid)
			break;
	}
	
	printf("%d %ld\n",n,(long)connectionsArray[n]->pid);

	switch(pid = fork()){
		case -1:	
			/* if fork failed */
			perror("Fork failed");
			exit(1);
		case 0:		
			/* Child-Process (WORKER) */
			execlp("./worker","./worker","-pp",connectionsArray[n]->connection_names[PARENT],"-pc",connectionsArray[n]->connection_names[CHILD],"-b",chbufferSize,NULL);
			break;
		default:
			/* Parent-Process (SERVER) */
			connectionsArray[n]->pid = pid;
			break;
	}

	struct pollfd fdarray[2];

	for(int i=0;i<2*numOfWorkers;i++)
		fdarray[i].fd = -1;

	int i=0,wfd,rfd;
	while(1){
		wfd = open(connectionsArray[n]->connection_names[PARENT],O_WRONLY);
		rfd = open(connectionsArray[n]->connection_names[CHILD],O_RDONLY|O_NONBLOCK);
		// printf("1. %s\n", connectionsArray[i]->connection_names[PARENT]);
		int j=0;
		while(j<2 && fdarray[j].fd>2) j++;
		if(j==2) break;

		if(wfd<0){
			if(errno==ENXIO)
				continue;
			else
				fprintf(stderr,"Pipe %s didn't open : %s \n",connectionsArray[n]->connection_names[PARENT],strerror(errno));
		}else{
			fdarray[0].fd = wfd;
			fdarray[0].events = POLLOUT;
		}

		if(rfd<0){
			if(errno==ENXIO)
				continue;
			else
				fprintf(stderr,"Pipe %s didn't open : %s \n",connectionsArray[n]->connection_names[PARENT],strerror(errno));
		}else{
			fdarray[1].fd = rfd;
			fdarray[1].events = POLLIN;
		}
		i++;		
		i=i%2;
	}


	int finished[2] ,sendtoPipe[1],totalSize_tobeSent[1],numOfChars_toSent[1];
	int rc;
	char ** Statistics = malloc(1*sizeof(char *));
	int totalSize_tobeRead[1];
	int sizeofReads[1],sizeindex[1];
	char **sizebuffer = malloc(sizeof(char*)*1);

	for(int i=0;i<1;i++){
		totalSize_tobeRead[i] = -1;
		sizeofReads[i] = 0;
		sizeindex[i] = 0;
		sizebuffer[i] = malloc(sizeof(char)*50);
		Statistics[i] = malloc(sizeof(char));
		strncpy(Statistics[i],"",1);
		// size = strlen(connectionsArray[n]->subdirs_concat)+1;
		// connectionsArray[n]->subdirs_concat = insertSizetoMessage(connectionsArray[n]->subdirs_concat,size);
		finished[i] = False;
		sendtoPipe[i] = 0;
		totalSize_tobeSent[i] = strlen(connectionsArray[n]->subdirs_concat)+1;
		numOfChars_toSent[i] =bufferSize;
	}

	for(int i=0;i<2;i++)
		finished[i] = False;



	/* ---- Passing the name of subdirs to workers ------- */
	int written;
	int TransmitingData=1;
	while(TransmitingData){

		int j=0;
		while(j<2 && finished[j]) j++;
		if(j==2)	break;

		rc = poll(fdarray,2,-1);

		if(rc==0){
			perror("Server: poll timed-out ");
			break;
		}else if(rc>0){
		
			for(int f=0;f<2;f++){

				if(f==0 && (fdarray[f].revents & POLLOUT) && finished[f]!=True) {
					
					if(sendtoPipe[f] >= totalSize_tobeSent[f] ){
						finished[f] = True;
						continue;
					}
					char tempBuffer[numOfChars_toSent[f]];
					strncpy(tempBuffer,connectionsArray[f]->subdirs_concat+sendtoPipe[f],numOfChars_toSent[f]);
					written = write(fdarray[f].fd,tempBuffer,numOfChars_toSent[f]);
					sendtoPipe[f]+=written;
					if((totalSize_tobeSent[f]-sendtoPipe[f])<bufferSize)
						numOfChars_toSent[f] = totalSize_tobeSent[f]-sendtoPipe[f];
				
				}

				if(f==1 && (fdarray[f].revents & POLLIN) && finished[f]!=True) {

					printf("Server: Smth to read\n");
					int read_index = 0;
					char buffer[bufferSize];
					int bytesRead=read(fdarray[f].fd,buffer,bufferSize);

					switch(bytesRead){
						case -1:
							if(errno == EAGAIN)
								break;
							if(errno == EINTR)
								printf("STOP\n");
							break;
						case 0:	break;
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

	printStatistics(Statistics,numOfWorkers);

	for(int i=0;i<1;i++){
		free(sizebuffer[i]);
		free(Statistics[i]);
	}

	free(sizebuffer);
	free(Statistics);
}