#include "utils.h"

#define DISEASE 1
#define COUNTRY 0

typedef struct Patient{

	char * id;
	char * firstName;
	char * lastName;
	char * country;
	char * disease;
	char * entryDate;
	char * exitDate;
	
}Patient;

typedef struct heapData{

	char * data;
	int heapKey;

}heapData;


/*---------------- Patient functions -----------------*/

Patient * createPatient(char * id,char * firstName,char * lastName ,char * disease,char * country,char * entryDate,char * exitDate);
void print_patientInfo(void * p);
void destroyPatient(Patient * p);
int countryPatients(void * patient,char * country); 
int diseasePatients(void * patient,char * disease);
int idComparator(void * patient,void * id);
int hospitalized(void * patient,char * null);

/*--------------Link functions between heap and patient -----------------*/

heapData * createHeapData(char * data,int key);
int heapComparator(void * value1,void * value2);

/* --------------- Statistics functions ---------------*/

int globalDiseaseStats(HashTable * hashtable,char * date1,char * date2);
int insertPatient(HashTable * hashtable1,HashTable * hashtable2,List * list,RBTNode ** treeRoot,FILE * file);
int diseaseFrequency(HashTable * hashtable,char * viusName,char * country,char * date1,char * date2);
int topK(int k,HashTable * dht,char * country,char * date1,char * date2,int diseaseORcountry);
int informPatient(RBTNode * treeRoot,char * id,char * exitDate);
int numCurrentPatients(HashTable * hashtable,char * disease);
