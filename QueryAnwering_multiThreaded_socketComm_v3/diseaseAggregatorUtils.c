#include "patientData.h"

void listCountries(int numOfWorkers,npipe_connection **connectionsArray){

	printf("\n");
	for(int i=0;i<numOfWorkers;i++){
		for(int d=0;d<connectionsArray[i]->numOfSubdirs;d++){
			char country[50],buffer[50];buffer[0] = '\0';
			strcat(buffer,connectionsArray[i]->subdirectories[d]);
			getCountry(buffer,country);
			printf("%s %ld \n",country,(long)connectionsArray[i]->pid);
		}
	}
}


void replaceChild(pid_t deadpid,int numOfWorkers,npipe_connection **connectionsArray,char * chbufferSize,struct pollfd * fdarray,char **portAndIP){
	/* Function that runs when catching SIGCHLD */
	pid_t pid;
	int n,bufferSize=atoi(chbufferSize);
	for(n=0;n<numOfWorkers;n++){
		if(connectionsArray[n]->pid == deadpid)
			break;
	}

	/* Creating new process */
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
			/* Parent-Process (PARENT) */
			connectionsArray[n]->pid = pid;
			break;
	}

	/* --- Closing old file descriptors --- */
	int wfd;
	close(fdarray[n].fd);
	fdarray[n].fd = -1;

	/* --- Opening new file descriptors --- */
	int i=0;
	while(1){
		wfd = open(connectionsArray[n]->connection_names[PARENT],O_WRONLY);

		int j=0;
		while(j<numOfWorkers && fdarray[j].fd>2) j++;
		if(j==numOfWorkers) break;

		if(wfd<0){
			if(errno==ENXIO)
				continue;
			else
				fprintf(stderr,"Pipe %s didn't open : %s \n",connectionsArray[i]->connection_names[PARENT],strerror(errno));
		}else{
			fdarray[n].fd = wfd;
			fdarray[n].events = POLLOUT;
		}
	}

	/* Start communication */
	char * subdirs_toSent[1];
	subdirs_toSent[0] = connectionsArray[n]->subdirs_concat;
	char country[100];
	getCountry(connectionsArray[n]->subdirectories[0],country);

	if(communicateWithWorkers(subdirs_toSent,1,&fdarray[n],bufferSize,connectionsArray,NULL)!=NULL)
		fprintf(stderr,"Master communication with worker failed\n");
	
	if(communicateWithWorkers(portAndIP,1,&fdarray[n],bufferSize,connectionsArray,NULL)!=NULL)
		fprintf(stderr,"Master communication with worker failed\n");

}


char ** communicateWithWorkers(char **toSend,int init_numOfWorkers,struct pollfd * ffdarray,int bufferSize,npipe_connection **connectionsArray,char * withCountry){
	/* Main function that writes data of array toSend to a pipe and returns read data from pipe in an array toRead  */

	struct pollfd * fdarray;
	int numOfWorkers = init_numOfWorkers;
	fdarray = ffdarray;

	/* --------- counters initialaization ------- */
	int finished[numOfWorkers],sendtoPipe[numOfWorkers],totalSize_tobeSent[numOfWorkers],numOfChars_toSent[numOfWorkers];
	char **sizebuffer = malloc(sizeof(char*)*numOfWorkers);

	for(int i=0;i<numOfWorkers;i++){
		sizebuffer[i] = malloc(sizeof(char)*50);
		finished[i] = False;
		sendtoPipe[i] = 0;
		totalSize_tobeSent[i] = strlen(toSend[i])+1;
		numOfChars_toSent[i] = bufferSize;
		finished[i] = False;
	}

	int written,pipe_index,rc;
	int TransmitingData=1;

	/* ---- Read and write data to pipe ----- */
	while(TransmitingData){

		int j=0;
		while(j<numOfWorkers && finished[j]) j++;
		if(j==numOfWorkers)	break;

		rc = poll(fdarray,numOfWorkers,-1);	// poll() checks file descs

		if(rc==0){
			perror("Server: poll timed-out ");
			break;
		}else if(rc>0){
		
			for(int f=0;f<numOfWorkers;f++){	
				pipe_index = f;

				if((fdarray[pipe_index].revents & POLLOUT) && finished[f]!=True) {	// case there are bytes to write to a pipe
					
					if(sendtoPipe[f] >= totalSize_tobeSent[f] ){ 	// if message size has been read then finishes
						finished[f] = True;
						continue;
					}
					char tempBuffer[numOfChars_toSent[f]];
					strncpy(tempBuffer,toSend[f]+sendtoPipe[f],numOfChars_toSent[f]);
					written = write(fdarray[pipe_index].fd,tempBuffer,numOfChars_toSent[f]);
					sendtoPipe[f]+=written;
					if((totalSize_tobeSent[f]-sendtoPipe[f])<bufferSize)
						numOfChars_toSent[f] = totalSize_tobeSent[f]-sendtoPipe[f];
				
				}

			}
		}else if(rc==-1){
			printf("%s\n",strerror(errno) );
			exit(1);
		}
	}

	for(int i=0;i<numOfWorkers;i++)
		free(sizebuffer[i]);
	free(sizebuffer);
	if(ffdarray!=fdarray)
		free(fdarray);

	return NULL;
}


void ReadAndPrintSummaryStatistics(int numOfWorkers,struct pollfd * fdarray,int bufferSize,pid_t senderpid,npipe_connection **connectionsArray){
	/* Function that reads statistics from all workers and prints them (used for SIGUSR1) */


	/* Allocating memory */
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

	/* Loop for reading messages from given filedscs */
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
	printStatistics(Statistics,numOfWorkers);

	for(int i=0;i<1;i++){
		free(sizebuffer[i]);
		free(Statistics[i]);
	}
	free(sizebuffer);
	free(Statistics);
}
