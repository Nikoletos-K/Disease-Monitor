#include "whoServer.h"

workerData ** newWorker(workerData **workersArray,int port,char * IP,int * arraySize){
	/* Inserting new worker to workersArray */

	int search=0;
	for(int i=0;i<*arraySize;i++){
		if(port!=workersArray[i]->port)
			search++;
		else
			return NULL;
	}

	workersArray = realloc(workersArray,(*arraySize+1)*sizeof(workerData *));
	workersArray[*arraySize] = malloc(sizeof(workerData));
	workersArray[*arraySize]->port = port;
	workersArray[*arraySize]->index = *arraySize;
	workersArray[*arraySize]->IP = strdup(IP);
	(*arraySize)++;
	return workersArray;
}

workerData ** removeUnusedPort(int array_position,workerData **array,int * arraySize){
	/* Removing worker that failed connection */

	workerData * temp = array[array_position];
	array[array_position] = array[*arraySize-1];
	free(temp->IP);
	free(temp);
	(*arraySize)--;
	return array;
}


void responcetoClient(int fd,char ** results,int arraySize){
	/* Forwarding results to qlient */

	char * clientMessage = malloc(1);
	clientMessage[0] = '\0';
	char numMes[10];numMes[0] = '\0';
	sprintf(numMes,"%d",arraySize);
	clientMessage = realloc(clientMessage,strlen(numMes)+2);
	strcat(clientMessage,numMes);
	strcat(clientMessage,"#");

	for(int i=0;i<arraySize;i++){
		clientMessage = realloc(clientMessage,strlen(clientMessage)+strlen(results[i])+2);
		strcat(clientMessage,results[i]);
		strcat(clientMessage,"@");
	}

	clientMessage = insertSizetoMessage(clientMessage,strlen(clientMessage)+1);

	/* Setting up poll for qlient */
	struct pollfd fdarray[1];
	fdarray[0].fd = fd;
	fdarray[0].events = POLLOUT;

	/* Sending data to socket */
	sendtoSocket(&clientMessage,1,fdarray,SOCKET_CHUNK);
	free(clientMessage);
}
	
void sendtoSocket(char **toSend,int init_numOfWorkers,struct pollfd * ffdarray,int bufferSize){
	/* Transmiting data to socket */

	int numOfWorkers;
	struct pollfd * fdarray;
	numOfWorkers = init_numOfWorkers;
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

	/* ---- Read and write data to socket ----- */
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
					
					if(sendtoPipe[f] >= totalSize_tobeSent[f] ){
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

}

cbuffer_Data * copyData(cbuffer_Data * tocopy){
	/* Copying data from cyclic buffer,for not locking it and losing time*/ 

	cbuffer_Data * temp = malloc(sizeof(cbuffer_Data));
	temp->fileDesc = tocopy->fileDesc;
	temp->statORquery = tocopy->statORquery;
	temp->clientORworker = tocopy->clientORworker;
	temp->empty = tocopy->empty;
	temp->IP = tocopy->IP;
	return temp;
}

int bind_on_port(int socket,short port){
	/* As in slides */
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port);
	return bind(socket,(struct sockaddr *) &server,sizeof(server));
}

char * readfromSocket(int bufferSize,int readfd){
	/* Reading data from a socket */

	/* Variables declaration */
	char * pipeRead = malloc(1);
	int bytesRead,sizeofReads=0,ReadfromParent=1,index=0,packageSize = -1;
	char size[50];
	while(ReadfromParent){

		char buffer[bufferSize];
		bytesRead=read(readfd,buffer,bufferSize);
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
				strncpy(pipeRead+sizeofReads,buffer,bytesRead);
				sizeofReads += bytesRead;
				break;
		}

		if(packageSize != -1 && sizeofReads>=packageSize+index+1)	//if finish break
			break;
				
		if(packageSize==-1 && bytesRead>0){		// branch for reading initial size

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
	return pipeRead;	
}


char ** readFromMultipleSockets(int numOfWorkers,struct pollfd * fdarray,int bufferSize){

	/* ---- initialization of counters ---- */
	int totalSize_tobeRead[numOfWorkers];
	int sizeofReads[numOfWorkers],sizeindex[numOfWorkers];
	char **sizebuffer = malloc(sizeof(char*)*numOfWorkers);
	char ** Statistics = malloc(numOfWorkers*sizeof(char *));
	int finished[numOfWorkers];
	
	/* --- preparing data to pass to the pipe ---- */
	for(int i=0;i<numOfWorkers;i++){
		Statistics[i] = malloc(sizeof(char));
		strncpy(Statistics[i],"",1);
		finished[i] = False;
		totalSize_tobeRead[i] = -1;
		sizeofReads[i] = 0;
		sizeindex[i] = 0;
		sizebuffer[i] = malloc(sizeof(char)*50);
	}

	/* Loop that runs until all data are passed and to the pipe */
	int TransmitingData=1,rc;
	while(TransmitingData){

		int j=0;
		while(j<numOfWorkers && finished[j]) j++;
		if(j==numOfWorkers)	break;

		rc = poll(fdarray,numOfWorkers,-1); // calling poll
		
		if(rc==0){
			perror("Server: poll timed-out ");
			break;
		}else if(rc>0){
		
			for(int f=0;f<numOfWorkers;f++){	
				if((fdarray[f].revents & POLLIN) && finished[f]!=True) {	// case that aggregator can read from the pipe

					char buffer[bufferSize];
					int bytesRead=read(fdarray[f].fd,buffer,bufferSize);	// read and checking the return values
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
			
					if(totalSize_tobeRead[f] != -1 && sizeofReads[f]>=totalSize_tobeRead[f]+sizeindex[f]+1){	// checking if read the initial message size
						finished[f]=True;
						continue;
					}
							
					if(totalSize_tobeRead[f]==-1 && bytesRead>0){	// read first bytes that depict size 

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
	for(int i=0;i<numOfWorkers;i++)
		free(sizebuffer[i]);
	free(sizebuffer);

	return Statistics;
}


int printResults(char ** results,int commandCode,int numOfWorkers){
	/* Function that splits and prints every responce message from workers that provoked from a user command */

	char * cases[4] = {"0-20","21-40","41-60","60+"};
	char * token;
	for(int i=0;i<numOfWorkers;i++){
		char * errorbuffer = malloc((strlen(results[i])+1)*sizeof(char));errorbuffer[0] = '\0';
		strcpy(errorbuffer,results[i]);
		char * token = strtok(errorbuffer,"!");

		if((token=strtok(NULL,"!")) != NULL ){
			fprintf(stdout,"%s\n",token);
			free(errorbuffer);
			return -1;
		}
		free(errorbuffer);
	}

	switch(commandCode){
		case TOPK_AGE_RANGES:
		{	
			char * class;

			for(int i=0;i<numOfWorkers;i++){				
				token = strtok(results[i],"%");
				token = strtok(NULL,"$");
				while(token!=NULL){
					class = strtok(NULL,"%");
					if(class==NULL) break;
					fprintf(stdout,"%s: %s%%\n",cases[atoi(token)],class);
					token = strtok(NULL,"$");
				}
			}
			break;
		}
		case SEARCH_PATIENT:
		{	
			int found=0;
			for(int i=0;i<numOfWorkers;i++){
				token = strtok(results[i],"%");
				token = strtok(NULL,"%");
				if(!strcmp(token,"-")) continue;
				else{
					found=1;
					fprintf(stdout,"%s\n",token);
				}
			}

			if(!found)
				fprintf(stdout,"This id doesn't belong to any country \n");
			break;
		}
		case NUM_PDISCHARGES:
		case NUM_PADMISSIONS:
		{
			for(int i=0;i<numOfWorkers;i++){
				token = strtok(results[i],"%");
				token = strtok(NULL,"%");
				while(token!=NULL){
					if(strcmp(token,"-"))
						fprintf(stdout,"%s\n",token);
					token = strtok(NULL,"%");			
				}
			}
			break;
		}
		case DISEASE_FREQUENCY:
		{
			int total=0;
			for(int i=0;i<numOfWorkers;i++){
				token = strtok(results[i],"%");
				token = strtok(NULL,"%");
				total+= atoi(token);
			}
			fprintf(stdout,"%d\n",total);
			break;
		}
		default:
			break;
	}
	return 1;
}