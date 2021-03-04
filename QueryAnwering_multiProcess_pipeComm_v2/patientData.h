#include "utils.h"

#define DISEASE 1
#define COUNTRY 0

typedef struct Patient{

	char * id;
	char * firstName;
	char * lastName;
	char * disease;
	char * entryDate;
	char * exitDate;
	char * country;	
	int age;

}Patient;

typedef struct diseaseStat{

	char diseasename[50];
	int ageClasses[4];

}diseaseStat;


// typedef struct heapData{

// 	char * data;
// 	int heapKey;

// }heapData;


/*---------------- Patient functions -----------------*/

Patient * createPatient(char * id,char * firstName,char * lastName ,char * disease,int age,char * entryDate,char * country);
void print_patientInfo(void * p);
void destroyPatient(void * patient);
void infromExitDate(Patient * p,char * exitDate);
// int countryPatients(void * patient,char * country); 
// int diseasePatients(void * patient,char * disease);
int idComparator(void * patient,void * id);
int ageCondition(void * p);
// int hospitalized(void * patient,char * null);

/*--------------Link functions between heap and patient -----------------*/

// heapData * createHeapData(char * data,int key);
// int heapComparator(void * value1,void * value2);

/* --------------- Statistics functions ---------------*/

// int globalDiseaseStats(HashTable * hashtable,char * date1,char * date2);
// int insertPatient(HashTable * hashtable1,HashTable * hashtable2,List * list,RBTNode ** treeRoot,FILE * file);
// int diseaseFrequency(HashTable * hashtable,char * viusName,char * country,char * date1,char * date2);
// int topK(int k,HashTable * dht,char * country,char * date1,char * date2,int diseaseORcountry);
// int informPatient(RBTNode * treeRoot,char * id,char * exitDate);
// int numCurrentPatients(HashTable * hashtable,char * disease);

/* ------- diseaseAggregator functions -----------------*/
void printStatistics(char **arrayStat,int numOfWorkers);
void listCountries(int numOfWorkers,npipe_connection **connectionsArray);
void ReadAndPrintSummaryStatistics(int numOfWorkers,struct pollfd * fdarray,int bufferSize,pid_t senderpid,npipe_connection **connectionsArray);
void catchSIGCHILD(int signo);
void replaceChild(pid_t deadpid,int numOfWorkers,npipe_connection **connectionsArray,char * chbufferSize);
void catchSIGUSR1andPid(int signo,siginfo_t *info,void *data);
/*-------- worker ------------*/
char * sendSummaryStatistics(HashTable * hashtable,char * toPipe);
char * concat_toMessage(char ***Statistics_toPipe,int numOfDirs,int *numOfFiles);
int CalculateAndSendStatistics(int newfiles,int *numOfFiles);
void catchSIGUSR1(int signo);
void catchSIGINT_OR_SIGQUIT(int signo);
#define NEW_FILES 1
#define INITIAL_FILES 0