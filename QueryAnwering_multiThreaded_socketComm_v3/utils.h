#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "RBTree.h"
#include "HashTable.h"
#include "maxHeap.h"

#define DATE_BUFFER sizeof("DD-MM-YYYY")
#define NAME_BUFFER 20
#define True 1
#define False 0
#define PARENT 0
#define CHILD 1
#define DEMON 1


typedef enum inputArgs{
	w,b,i,s,p
}inputArgs;

typedef enum errors{
	noError=1,inputError=-1,commandLine=-2,dateMissing=-3,sameRecord=-4
}errors;

typedef enum promptFlags{

	DISEASE_FREQUENCY=1,TOPK_AGE_RANGES=2,
	SEARCH_PATIENT=3,NUM_PDISCHARGES=4,NUM_PADMISSIONS=5,
	INPUT_ERROR=-1,EXIT=0,NO_INPUT=-2,LIST=6

}promptFlags;

int error;

typedef struct npipe_connection{

	char **connection_names;
	int index;
	char **subdirectories;
	int * pipe_filedesc;
	int numOfSubdirs;
	char * subdirs_concat;
	pid_t pid;

} npipe_connection;

npipe_connection * create_pipeConnection(int index);
void destroy_pipeConnection(npipe_connection * pipe);
void insert_subdirectory(npipe_connection * pipe,char *subdirectory);
char * insertSizetoMessage(char * str,int size);
void set_pid(pid_t pid,npipe_connection * pipe);
void getCountry(char * path,char * country);

/* ---------------------- Utility - Functions -------------------------*/

void printString(void * i);
int input(char * inputStr);
int errorHandler(int errorCode,char * errorStr);
int dateCompare(const  void * date1,const void * date2);
int dateComparator(const  void * date1,const void * date2);
int dateExists(char * date1);
int Prompt(char * inputStr);
void printOptions();
int integerComparator(const void* v1,const void* v2);
int stringComparator(const void * v1,const void * v2);
int onlyLettersandNumbers(char * str);
void printInteger(void * i);
void insertDelimeters(char * str,char delim);