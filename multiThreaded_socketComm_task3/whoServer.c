#include "whoServer.h"

/* Mutexes and condition variables (as proposed in slides) */
pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;

/* Global declaration of cyclic buffer */
cyclic_buffer * cbuffer;

/* Number of file descriptors in cb */
int numofFds;

/* Array that stores workers information */
workerData **workersArray;
int workerArraySize=0;

/* Signal flag */
volatile sig_atomic_t SIGINT_flag=0;


void * Thread(void * argp){
	/* Threads function */

	while(1){	/* Infinite loop because threads are reused */

		/* Checking and consuming a file descriptor from cyclic buffer */
		cbuffer_Data * temp =  obtain(cbuffer);
		pthread_cond_signal(&cond_nonfull);	// signal that one fd has consumed and so buffer not full

		/* 2 cases - fd for statistics or fd for query */
		if(temp->statORquery==STAT && temp->clientORworker==WORKER){	// Case file desc coming from worker for statistics

			/* Reading port */
			int workerPort;
			read(temp->fileDesc,&workerPort,sizeof(int));
			
			/* Lock because multithread data will be changed */
			pthread_mutex_lock(&mtx);
			workerData ** inserted;
			if((inserted=newWorker(workersArray,workerPort,temp->IP,&workerArraySize))!=NULL)	// if this port not in array
				workersArray = inserted;	// inserting new workers data
			pthread_mutex_unlock(&mtx);

			/* Reading statistics */
			char * socketStat = readfromSocket(256,temp->fileDesc);
			
			/* Locking stdout for clear printing*/
			flockfile(stdout);
			fprintf(stdout,"\nSTATISTICS\n");
			// printStatistics(&socketStat,1);
			funlockfile(stdout);

			/* Free memory */
			free(socketStat);
			free(temp->IP);
			close(temp->fileDesc);

		}else if(temp->statORquery==QUERY && temp->clientORworker==CLIENT){		// Case fd from a qlient

			/* Reading command-query */
			char * command = malloc(100);
			read(temp->fileDesc,command,100);

			/* Forwarding command to all workers and printing-forwarding results to qlient */
			forwardCommand(command,workersArray,&workerArraySize,temp->fileDesc);
			close(temp->fileDesc);
		}
		free(temp);
	}
	pthread_exit(NULL);
}

cyclic_buffer * initialize(cyclic_buffer * cbuffer,int bufferSize){
	/* As in slides but buffer consisted from structs and not integers*/

	cbuffer = malloc(sizeof(cyclic_buffer));
	cbuffer->start = 0;
	cbuffer->end = -1;
	cbuffer->count = 0;
	cbuffer->size = bufferSize;
	cbuffer->buffer = malloc(bufferSize*sizeof(cbuffer_Data*));
	for(int i=0;i<bufferSize;i++){
		cbuffer->buffer[i] =  malloc(sizeof(cbuffer_Data));
		cbuffer->buffer[i]->empty = EMPTY;
	}
	return cbuffer;
}

void place(cyclic_buffer * cbuffer,int newFD,int statORquery,int clientORworker,char * IP){
	/* Almost as in slides */

	pthread_mutex_lock(&mtx);
	while(cbuffer->count >= cbuffer->size)
		pthread_cond_wait(&cond_nonfull,&mtx);

	cbuffer->end = (cbuffer->end+1) % (cbuffer->size);
	cbuffer->buffer[cbuffer->end]->fileDesc = newFD;
	cbuffer->buffer[cbuffer->end]->statORquery = statORquery;
	cbuffer->buffer[cbuffer->end]->clientORworker = clientORworker;
	cbuffer->buffer[cbuffer->end]->empty = NOTEMPTY;
	cbuffer->buffer[cbuffer->end]->IP = IP;
	cbuffer->count++;
	pthread_mutex_unlock(&mtx);
}

cbuffer_Data * obtain(cyclic_buffer * cbuffer){
	/* Almost as in slides */

	pthread_mutex_lock(&mtx);
	while(cbuffer->count <= 0)
		pthread_cond_wait(&cond_nonempty,&mtx);
	
	cbuffer_Data * temp = copyData(cbuffer->buffer[cbuffer->start]);
	cbuffer->buffer[cbuffer->start]->empty = EMPTY;
	cbuffer->start = (cbuffer->start+1) % (cbuffer->size);
	(cbuffer->count)--;
	pthread_mutex_unlock(&mtx);
	return temp;
}

int main(int argc,char **argv){

	/* Setting up handler for SIGINT */
	static struct sigaction actSIGINT;
	actSIGINT.sa_handler = catchSIGINT_OR_SIGQUIT;
	sigfillset(&(actSIGINT.sa_mask));
	sigaction(SIGINT,&actSIGINT,NULL);

	fprintf(stdout,"----- whoSERVERs CONNECTIONS INFORMATION -----\n");
	fprintf(stdout,"-> SERVER starts\n");
	int i=1;
	int queryPortNum,statisticsPortNum,numThreads,bufferSize;
	
	/* Reading from command line */ 
	while(argv[i]!=NULL){
		if(!strcmp(argv[i],"-q"))
			queryPortNum = atoi(argv[i+1]);
		else if(!strcmp(argv[i],"-s"))
			statisticsPortNum = atoi(argv[i+1]);
		else if(!strcmp(argv[i],"-b"))
			bufferSize = atoi(argv[i+1]);
		else if(!strcmp(argv[i],"-w"))
			numThreads = atoi(argv[i+1]);
		i++;			
	}

	/* Initializing mutexes and cond variables */
	pthread_mutex_init(&mtx,0);
	pthread_cond_init(&cond_nonempty,0);
	pthread_cond_init(&cond_nonfull,0);

	cbuffer = initialize(cbuffer,bufferSize);
	int err;

	/* ---- Thread creation ----- */
	pthread_t threadArray[numThreads];

	for(int t=0;t<numThreads;t++){	
		if((err = pthread_create(&threadArray[t],NULL,Thread,NULL))){
			errorHandler(err,"pthread_create");
			exit(EXIT_FAILURE);
		}
	}


	/* --- Creating sockets as proposed in slides ---- */
	
	/* -> Statistics socket  ------ */
	struct sockaddr_in server,client;
	socklen_t clientlen = sizeof(client);
	struct sockaddr * serverptr = (struct sockaddr *) &server;
	struct sockaddr * clientptr = (struct sockaddr *) &client;
	struct hostent * rem;


	int statisticsSocket,newSocket;
	if((statisticsSocket = socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("Socket creation failed");fflush(stdout);
		exit(EXIT_FAILURE);
	}	
	
	fprintf(stdout,"- whoSERVERs statistics socket creation: SUCCESS\n");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(statisticsPortNum);

	if(bind(statisticsSocket,serverptr,sizeof(server))<0){
		perror("Server socket bind failed (statistics)");fflush(stdout);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout,"- whoSERVERs statistics socket bind: SUCCESS\n");

	/* For backlog I use SOMAXCONN */
	if(listen(statisticsSocket,SOMAXCONN)<0){
		perror("Listen to statistics sockets failed");fflush(stdout);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout,"- whoSERVERs statistics socket listen: SUCCESS\n");

	/* -> Query socket  ------ */
	struct sockaddr_in qserver,qclient;
	socklen_t qclientlen = sizeof(qclient);
	struct sockaddr * qserverptr = (struct sockaddr *) &qserver;
	struct sockaddr * qclientptr = (struct sockaddr *) &qclient;

	int querySocket;
	if((querySocket = socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("Socket creation failed");fflush(stdout);
		exit(EXIT_FAILURE);
	}	

	fprintf(stdout,"- whoSERVERs query socket creation: SUCCESS\n");
	
	qserver.sin_family = AF_INET;
	qserver.sin_addr.s_addr = htonl(INADDR_ANY);
	qserver.sin_port = htons(queryPortNum);

	if(bind(querySocket,qserverptr,sizeof(qserver))<0){
		perror("Server socket bind failed (querySocket)");fflush(stdout);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout,"- whoSERVERs query socket bind: SUCCESS\n");

	if(listen(querySocket,SOMAXCONN)<0){
		perror("Listen to querySocket failed");fflush(stdout);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout,"- whoSERVERs query socket listen: SUCCESS\n");

	/* Setting up poll for manipulating events in the two sockets */
	struct pollfd fdarray[2];
	fdarray[0].fd = statisticsSocket;
	fdarray[0].events = POLLIN;
	fdarray[1].fd = querySocket;
	fdarray[1].events = POLLIN;
	int event=0;

	fprintf(stdout,"-> whoSERVER is ready to recieve requests\n");
	fprintf(stdout,"----------------------------------------------\n\n");

	/* Poll loop */
	while(1){

		event = poll(fdarray,2,-1);

		if(event>0){

			if((fdarray[0].revents & POLLIN)){	// STATISTICS EVENT

				if((newSocket=accept(statisticsSocket,clientptr,&clientlen))<0){
					perror("Accept statistics socket");
					exit(EXIT_FAILURE);
				}

				if((rem = gethostbyaddr((char *)&client.sin_addr.s_addr,sizeof(client.sin_addr.s_addr),client.sin_family))==NULL){
					herror("Server-gethostbyaddr");
					exit(EXIT_FAILURE);
				}

				/* Storing and forwarding IP from the accepted socket */
				char * ipbuffer = malloc(INET_ADDRSTRLEN);
				while((inet_ntop(AF_INET,&(client.sin_addr),ipbuffer,INET_ADDRSTRLEN))==NULL){
					if(errno==EAGAIN)
						continue;
					perror("inet_ntop");
					exit(EXIT_FAILURE);
				}

				/* Inserting in cyclic buffer */
				place(cbuffer,newSocket,STAT,WORKER,ipbuffer);

				/* Activating one thread to serve this fd */
				pthread_cond_signal(&cond_nonempty);
			}

			if((fdarray[1].revents & POLLIN)){	// CLIENT EVENT 

				if((newSocket=accept(querySocket,qclientptr,&qclientlen))<0){
					perror("Accept statistics socket");
					exit(EXIT_FAILURE);
				}

				if((rem = gethostbyaddr((char *)&qclient.sin_addr.s_addr,sizeof(qclient.sin_addr.s_addr),qclient.sin_family))==NULL){
					herror("Server-gethostbyaddr");
					exit(EXIT_FAILURE);
				}

				/* Inserting in cyclic buffer */
				place(cbuffer,newSocket,QUERY,CLIENT,NULL);

				/* Activating one thread to serve this fd */				
				pthread_cond_signal(&cond_nonempty);
			}

		}else if(event<0){

			if(SIGINT_flag){
				fprintf(stdout,"whoServer signal to finish\n");
				break;
			}
			perror("poll");
			exit(EXIT_FAILURE);
		}


		if(SIGINT_flag){
			fprintf(stdout,"whoServer signal to finish\n");
			break;
		}

	}

	/* Time to finish */
	close(statisticsSocket);
	close(querySocket);

	/* Killing threads */
	for(int t=0;t<numThreads;t++)
		pthread_kill(threadArray[t],SIGKILL);

	/* Freeing memory */
	for(int i=0;i<workerArraySize;i++){
		free(workersArray[i]->IP);
		free(workersArray[i]);
	}
	free(workersArray);
	for(int j=0;j<bufferSize;j++)
		free(cbuffer->buffer[j]);
	free(cbuffer->buffer);
	free(cbuffer);

	/* Destroying mutexes and vars */
	pthread_mutex_destroy(&mtx);
	pthread_cond_destroy(&cond_nonempty);
	pthread_cond_destroy(&cond_nonfull);

return 0;
}

void catchSIGINT_OR_SIGQUIT(int signo){
	/*SIGINT handler */
	SIGINT_flag = 1;
}


void forwardCommand(char * command,workerData ** workersArray,int * workerArraySize,int qlientFd){
	/* Function that forwards command to workers,recieves answers,prints them and forwrds them to qlient */

	int socketArray_sendData[*workerArraySize];
	char commandBuffer[200];commandBuffer[0] = '\0';
	strcpy(commandBuffer,command);

	/* -------  Creating connections with workers --------- */
	struct sockaddr_in client;
	socklen_t clientlen = sizeof(client);
	struct sockaddr * clientptr = (struct sockaddr *) &client;

	struct hostent * foundhost;
	struct in_addr address;

	command = insertSizetoMessage(command,strlen(command)+1);
	insertDelimeters(command,'%');
	
	/* Locking mutex because workers array may be changed */
	pthread_mutex_lock(&mtx);

	for(int i=0;i<*workerArraySize;i++){

		if((socketArray_sendData[i] = socket(AF_INET,SOCK_STREAM,0))==-1){
			perror("Socket creation failed");
			exit(EXIT_FAILURE);
		}

		while((error=inet_pton(AF_INET,workersArray[i]->IP,&address))<=0){
			if(errno==EAGAIN)
				continue;
			perror("inet_pton");
			exit(EXIT_FAILURE);
		}

		if((foundhost = gethostbyaddr((const char *)&address,sizeof(address),AF_INET))==NULL){
			herror("gethostbyaddr");
			exit(EXIT_FAILURE);
		}

		client.sin_family = AF_INET;
		memcpy(&(client.sin_addr),foundhost->h_addr,foundhost->h_length);
		client.sin_port = htons(workersArray[i]->port);

		int noconnect=0;
		while(connect(socketArray_sendData[i],clientptr,clientlen)){
			
			/* If system could't connect (ECONNREFUSED) ,this means that the wanted port don't exist anymore
				so I remove this connection from workersArray */ 
			if(errno == ECONNREFUSED){
				workersArray = removeUnusedPort(i,workersArray,workerArraySize);
				close(socketArray_sendData[i]);
				socketArray_sendData[i] = -1;
				i--;
				noconnect=1;
				break;	
			}
			perror("whoServer connection to worker failed");
			exit(EXIT_FAILURE);
		}
		if(noconnect)	continue;

		/* Forward command to worker */
		write(socketArray_sendData[i],command,strlen(command)+1);
	}

	/* Setting up poll for workers file descs*/
	struct pollfd fdarray[*workerArraySize];
	for(int i=0;i<*workerArraySize;i++){
		if(socketArray_sendData[i]!=-1){
			fdarray[i].fd = socketArray_sendData[i];
			fdarray[i].events = POLLIN;
		}
	}
	pthread_mutex_unlock(&mtx);

	/* Reading results from every worker */
    char ** results = readFromMultipleSockets(*workerArraySize,fdarray,SOCKET_CHUNK);

    /* Sending results to qlient */
    responcetoClient(qlientFd,results,*workerArraySize);
 
    /* Printing results */
    char * token = strtok(command,"%");
    token = strtok(NULL,"%");
	flockfile(stdout);
	fprintf(stdout,"\nQUERY: %s\n",commandBuffer);
	fprintf(stdout,"RESULTS: \n");	
    printResults(results,Prompt(token),*workerArraySize);
	fflush(stdout);
	funlockfile(stdout);

	for(int i=0;i<*workerArraySize;i++){
		if(socketArray_sendData[i]!=-1)
			close(socketArray_sendData[i]);
		free(results[i]);
	}
	free(results);
	free(command);
}	