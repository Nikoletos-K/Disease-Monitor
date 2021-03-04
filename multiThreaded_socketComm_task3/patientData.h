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


typedef struct heapData{

	int data;
	int heapKey;

}heapData;


/*---------------- Patient functions -----------------*/

Patient * createPatient(char * id,char * firstName,char * lastName ,char * disease,int age,char * entryDate,char * country);
void print_patientInfo(void * p);
void destroyPatient(void * patient);
void infromExitDate(Patient * p,char * exitDate);
int countryPatients(void * patient,char * country); 
int diseasePatients(void * patient,char * disease);
int idComparator(void * patient,void * id);
int ageCondition(void * p);
int notexitedPatients(void * patient,char * exit);		// same with disease
char * getExitDate(void * p);

/*--------------Link functions between heap and patient -----------------*/

heapData * createHeapData(int data,int key);
int heapComparator(void * value1,void * value2);

/* ------- DiseaseAggregator functions -----------------*/

void printStatistics(char **arrayStat,int numOfWorkers);
void listCountries(int numOfWorkers,npipe_connection **connectionsArray);
void ReadAndPrintSummaryStatistics(int numOfWorkers,struct pollfd * fdarray,int bufferSize,pid_t senderpid,npipe_connection **connectionsArray);
void catchSIGCHILD(int signo,siginfo_t *info,void *data);
void replaceChild(pid_t deadpid,int numOfWorkers,npipe_connection **connectionsArray,char * chbufferSize,struct pollfd * fdarray,char **portAndIP);
void catchSIGUSR1andPid(int signo,siginfo_t *info,void *data);
char ** communicateWithWorkers(char **toSend,int init_numOfWorkers,struct pollfd * ffdarray,int bufferSize,npipe_connection **connectionsArray,char * withCountry);
int printResults(char ** results,int commandCode,int numOfWorkers);


/*-------- Worker ------------*/

#define NEW_FILES 1
#define INITIAL_FILES 0
char * sendSummaryStatistics(HashTable * hashtable,char * toPipe);
char * concat_toMessage(char ***Statistics_toPipe,int numOfDirs,int *numOfFiles);
int CalculateAndSendStatistics(int newfiles,int *numOfFiles,int writefd);
void catchSIGUSR1(int signo);
void catchSIGINT_OR_SIGQUIT(int signo);
char * executeCommand(char * command,char * result);
int diseaseFrequency(char * disease,char * date1,char * date2,char * country,HashTable * hashtable);
int discharges(char * disease,char * date1,char * date2,char * country,HashTable * hashtable);
char * topk(int k,char * disease,char * date1,char * date2,HashTable * hashtable,char * result);
void ageClassesfromTree(RBTNode * node,int * ageCounters,char * key1,char * key2,int (*ageClass)(void*),int (*comparator)(const void*,const void*));
void FindSpecificNodes(RBTNode * node,int * counter,valueType key1,valueType key2,int (*comparator)(valueType,valueType),char * (*getExitDate)(void *));
char * readfromPipe(int bufferSize,int readfd);