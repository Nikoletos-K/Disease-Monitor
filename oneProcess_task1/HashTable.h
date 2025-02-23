/* hashtable consists of buckets and buckets point to a memory that are stored HTrecords */
#include <unistd.h>

#define ERROR -1
#define NO_ERROR 1

typedef struct Bucket Bucket;

typedef struct HTRecord{
		
	void * value;
	char * key;
	int counter;

}HTRecord;

typedef struct Bucket{

	int records;		// counter of records saved in bucketMemory
	void * bucketMemory;	// bucket storage
	Bucket * next;

}Bucket;

typedef struct HashTable{
	
	size_t hashtableSize;
	size_t bucketSize;
	int records_perBucket;
	int numOfRecords;
	Bucket **buckets; 

}HashTable;


/*----------- HashTable functions ------------------*/ 

unsigned int hashFunction(char * str,unsigned int length);

// constructors
HashTable * create_HashTable(size_t hashtableSize,size_t bucketSize);
Bucket * createBucket(size_t bucketSize);
HTRecord * createHTRecord(char * key);

// insertion functions
int insert_toHashTable(void * value,char * HTkey,char * treeKey,HashTable * ht,int (*comparator)(char*,char*));
int insertRecord(void * htRecord,Bucket * bucket);
void increaseCounter(HTRecord * record);

// geters
void * getValue(HTRecord * record);
int getCounter(HTRecord * record);
int getNumOfRecords(HashTable * ht);

// search functions
void numOfRecordsBetweenKeys(HashTable * hashtable,char * date1,char * date2,int (*comparator)(char *,char *));
int findKeyData(HashTable * hashtable,char * wantedKey,char * date1,char * date2,int (*comparator)(char *,char *),char * funValue,int (*function)(void*,char*));

// destructors
void destroy_HashTable(HashTable * ht);
void destroyHashTable(HashTable * hashtable);

// print finction
int printConditionHT(HashTable * hashtable,int (*condition)(void*,char*));

