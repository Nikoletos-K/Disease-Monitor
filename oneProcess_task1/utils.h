#include "list.h"
#include "RBTree.h"
#include "HashTable.h"
#include "maxHeap.h"

#define DATE_BUFFER sizeof("DD-MM-YYYY")
#define NAME_BUFFER 20
#define True 1
#define False 0

typedef enum inputArgs{
	p,h1,h2,b
}inputArgs;

typedef enum errors{
	noError=1,inputError=-1,commandLine=-2,dateMissing=-3,sameRecord=-4
}errors;

typedef enum promptFlags{
	GLOBAL_DSTATS,DISEASE_FREQUENCY,
	TOPK_DISEASES,TOPK_COUNTRIES,INSERT_PATIENT,
	REC_PATIENT_EXIT,NUM_CUR_PATIENTS,EXIT
}promptFlags;

int error;

/* ---------------------- Utility - Functions -------------------------*/

int input(char * inputStr);
int errorHandler(int errorCode,char * errorStr);
int dateCompare(char * date1,char * date2);
int dateExists(char * date1);
int Prompt(char * inputStr);
void printOptions();
int integerComparator(void* v1,void* v2);
int stringComparator(char* v1,char* v2);