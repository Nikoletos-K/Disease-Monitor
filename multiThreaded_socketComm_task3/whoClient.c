#include "whoServer.h"

#include <pthread.h>
#define errorHandler(e,s) fprintf(stderr,"%s: %s\n",s,strerror(e))

int port;
char * IP;

/* Mutexes and variables that make the concurrent run of multiple threads */
/* This model is based is some parts in an pseudocode model for concurrent run in stackoverflow */
int waitingThreads=0;
int concurrentThreads=0;
int readyFlag=0;

pthread_mutex_t mtx;
pthread_cond_t cond;
pthread_cond_t lastThread_cond;	// cond to signal when ALL threads wait


void * connectionThread(void * argp){
	/* Threads function */

	pthread_mutex_lock(&mtx);
	if(readyFlag==0) {	//if main thread not ready
		waitingThreads++;	// inc counter 
		do{
			pthread_cond_signal(&lastThread_cond);	// signal cond of main thread
			pthread_cond_wait(&cond,&mtx);	// wait if not all numthreads ready to run
		} while(readyFlag==0);
		waitingThreads--;	// if thread proceeds from cond and main thread is ready dicrease counter
	}
	pthread_mutex_unlock(&mtx);

	/* ------ Preparing and setting connection to given port and ip --------- */
	struct hostent * foundhost;
	struct in_addr address;
	int serverSocket;
	struct sockaddr_in server;
	struct sockaddr * serveptr = (struct sockaddr *)&server;

	/* Establishing connection with Server */
	
	if((serverSocket = socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	int error;
	while((error=inet_pton(AF_INET,IP,&address))<=0){
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
		perror("Client socket connection failed");
		exit(EXIT_FAILURE);
	}
	
	/* ---- Writting command to server socket ------ */
	write(serverSocket,(char*)argp,strlen(argp)+1);

	/* ----- reading from socket results ----- */
	char * results = readfromSocket(SOCKET_CHUNK,serverSocket);

	/* ------ Preparing results to printable format ------- */
    char * numMes = strtok(results,"%");
    numMes = strtok(NULL,"#");
    int numMessages = atoi(numMes);
    char * results_per_message[numMessages];
    for(int i=0;i<numMessages;i++){
    	results_per_message[i] = malloc(1);
    	results_per_message[i][0] = '\0';
    }
    
    int i=0;
    numMes = strtok(NULL,"@");
    while(numMes!=NULL){
    	results_per_message[i] = realloc(results_per_message[i],strlen(numMes)+1);
    	strcpy(results_per_message[i],numMes);
    	i++;
    	numMes = strtok(NULL,"@");
    }

    /*-------- Printing results ---------*/
	flockfile(stdout);
	fprintf(stdout,"\n+-------------------------------------------------+\n");
	fprintf(stdout,"QUERY: %s\n",(char*)argp);
	char * token = strtok(argp," ");
	fprintf(stdout,"RESULTS: \n");
    printResults(results_per_message,Prompt(token),numMessages);
	fprintf(stdout,"+-------------------------------------------------+\n");
	fflush(stdout);
	funlockfile(stdout);

	/* ----- Releasing memory -------- */
	close(serverSocket);
	pthread_mutex_lock(&mtx);
	concurrentThreads--;
	pthread_cond_signal(&lastThread_cond);
	pthread_mutex_unlock(&mtx);
	for(int i=0;i<numMessages;i++)
		free(results_per_message[i]);
	free(results);
	free(argp);
	return 0;
}

int main(int argc,char **argv){

	fprintf(stdout,"------- whoClient INFORMATION -------\n");
	int i=1;
	FILE * queryFile;
	IP = malloc(50);IP[0] = '\0';
	int numThreads;

	/* ------ reaing command lines arguments ----- */
	while(argv[i]!=NULL){
		if(!strcmp(argv[i],"-q"))
			queryFile = fopen(argv[i+1],"r");
		else if(!strcmp(argv[i],"-sp"))
			port = atoi(argv[i+1]);
		else if(!strcmp(argv[i],"-sip"))
			strcpy(IP,argv[i+1]);
		else if(!strcmp(argv[i],"-w"))
			numThreads = atoi(argv[i+1]);
		i++;			
	}

	/* ----  mutex and cond variables initialazation ----- */
	pthread_mutex_init(&mtx,0);
	pthread_cond_init(&cond,0);
	pthread_cond_init(&lastThread_cond,0);

	int err,status;
	pthread_t * threadArray = malloc(numThreads*sizeof(pthread_t));


	/* ----- Reading every line from query file ------ */
	int lines=0;
	while(!feof(queryFile)){
		char * command = malloc(100);
		fscanf(queryFile,"%[^\n]\n",command);

		/* Creating thread for every line */
		if((err = pthread_create(&threadArray[lines],NULL,connectionThread,(void*)command))){
			errorHandler(err,"pthread_create");
			exit(EXIT_FAILURE);
		}
		concurrentThreads++;

		if(lines == numThreads-1){	// LINES equal to numthreads means waking up all ready threads to run
			fprintf(stdout,"Ready to send new queries\n");
			
			/* Waiting untill the last thread waits to the mutex or variable */
			pthread_mutex_lock(&mtx);
			while(waitingThreads<concurrentThreads)
				pthread_cond_wait(&lastThread_cond,&mtx);

			/* If all ready broadcast them all */
			if(waitingThreads!=0){
				readyFlag=1;
				pthread_cond_broadcast(&cond);
			}
			pthread_mutex_unlock(&mtx);

			/* Waiting threads to finish */
			lines=0;
			for(int t=0;t<numThreads;t++){	
				if((err = pthread_join(threadArray[t],(void**)&status))){
					errorHandler(err,"pthread_join");
					exit(EXIT_FAILURE);
				}
			}
			fprintf(stdout,"\n%d threads finished\n",numThreads);

			/* Restoring flags */
			pthread_mutex_lock(&mtx);
			if(waitingThreads==0)
				readyFlag=0;
			pthread_mutex_unlock(&mtx);

		}else
			lines++;

	}

	/* Same as above but in case that lines has been consumed */
	if(lines!=0){

			fprintf(stdout,"Ready to send new queries\n");
			pthread_mutex_lock(&mtx);
			while(waitingThreads<concurrentThreads)
				pthread_cond_wait(&lastThread_cond,&mtx);

			if(waitingThreads!=0){
				readyFlag=1;
				pthread_cond_broadcast(&cond);
			}
			pthread_mutex_unlock(&mtx);

			for(int t=0;t<lines	;t++){	
				if((err = pthread_join(threadArray[t],(void**)&status))){
					errorHandler(err,"pthread_join");
					exit(EXIT_FAILURE);
				}
			}
			fprintf(stdout,"\n%d threads finished\n",lines);			
			pthread_mutex_lock(&mtx);
			if(waitingThreads==0)
				readyFlag=0;
			pthread_mutex_unlock(&mtx);
	}

	/* Dstroying mutexes and free of allacated memory */
	pthread_mutex_destroy(&mtx);
	pthread_cond_destroy(&cond);
	pthread_cond_destroy(&lastThread_cond);	
	free(threadArray);
	free(IP);
	fclose(queryFile);
	fprintf(stdout,"-> whoClient finished\n");				
	pthread_exit(NULL);
	return 0;
}