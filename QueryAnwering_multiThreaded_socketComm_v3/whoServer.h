#include "patientData.h"
#include <pthread.h>
#define errorHandler(e,s) fprintf(stderr,"%s: %s\n",s,strerror(e))
#define CLIENT 1
#define WORKER 0
#define STAT 1
#define QUERY 0
#define EMPTY 0
#define NOTEMPTY 1
#define SOCKET_CHUNK 256

typedef struct {
	
	int empty;
	int fileDesc;
	int clientORworker;
	int statORquery;
	char * IP;

}cbuffer_Data;


typedef struct {
	
	int index;
	int port;
	char * IP;

}workerData;


typedef struct {
	
	cbuffer_Data **buffer;
	int start;
	int end;
	int count;
	int size;

}cyclic_buffer;


workerData ** newWorker(workerData **workersArray,int port,char * IP,int * arraySize);
cyclic_buffer * initialize(cyclic_buffer * cbuffer,int bufferSize);
void place(cyclic_buffer * cbuffer,int newFD,int statORquery,int clientORworker,char * IP);
void * Thread(void * argp);
int bind_on_port(int socket,short port);
cbuffer_Data * copyData(cbuffer_Data * tocopy);
cbuffer_Data * obtain(cyclic_buffer * cbuffer);
char * readfromSocket(int bufferSize,int readfd);
void sendCommand_toWorker(int fileDesc,char * cmdtosend);
char ** readFromMultipleSockets(int numOfWorkers,struct pollfd * fdarray,int bufferSize);
void forwardCommand(char * command,workerData ** workersArray,int * workerArraySize,int qlientFd);
void sendtoSocket(char **toSend,int init_numOfWorkers,struct pollfd * ffdarray,int bufferSize);
void responcetoClient(int fd,char ** results,int arraySize);
workerData ** removeUnusedPort(int array_position,workerData **array,int * arraySize);