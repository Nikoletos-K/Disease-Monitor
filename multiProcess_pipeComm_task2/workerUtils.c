#include "patientData.h"

void rbtStatistics(RBTNode * node,int * ageCounters,int (*ageClass)(void*)){
	int class;
	if(node==GUARD)
		return;
	rbtStatistics(node->left,ageCounters,ageClass);
	class=(*ageClass)(node->data);
	ageCounters[class]++; 
	rbtStatistics(node->right,ageCounters,ageClass);
}
char * sendSummaryStatistics(HashTable * hashtable,char * toPipe){


	int loops = 0;
	for(int i=0;i<hashtable->hashtableSize;i++){	// searches all hashtable

		if(hashtable->buckets[i]==NULL)
			continue;

		HTRecord * tempRecord;
		Bucket * tempBucket = hashtable->buckets[i];

		while(tempBucket!=NULL){	/* search the list of buckets */
			tempRecord = (HTRecord *) tempBucket->bucketMemory;

			while(loops<tempBucket->records){	/* search inside buckets */

				int ageArray[4];
				for(int i=0;i<4;i++) ageArray[i]=0;
				RBTNode * treeRoot = (RBTNode *) getValue(tempRecord);
				rbtStatistics(treeRoot,ageArray,ageCondition);
				char buffer[100];
				strcpy(buffer,(char*)tempRecord->key);

				for(int i=0;i<4;i++){
					char n[100];
					sprintf(n,"%d",ageArray[i]);
					strcat(buffer,"$");
					strcat(buffer,n);
				}
				strcat(buffer,"%");
				toPipe = realloc(toPipe,(strlen(toPipe)+strlen(buffer)+2)*sizeof(char));
				strcat(toPipe,buffer);
				if(loops < tempBucket->records-1){
					tempRecord++;
				}
				loops++;
			}
			tempBucket = tempBucket->next;
			loops=0;
		}
	}
	toPipe = realloc(toPipe,(strlen(toPipe)+2)*sizeof(char));
	strcat(toPipe,"%");
	return toPipe;
}

char * concat_toMessage(char ***Statistics_toPipe,int numOfDirs,int * numOfFiles){

	char * temp = strdup("");
	for(int d=0;d<numOfDirs;d++){
		if(Statistics_toPipe[d]==NULL) continue;
		temp = realloc(temp,(strlen(temp)+2)*sizeof(char));
		strcat(temp,"/");
		for(int f=0;f<numOfFiles[d];f++){			
			temp = realloc(temp,(strlen(temp)+strlen(Statistics_toPipe[d][f])+1)*sizeof(char));
			strcat(temp,Statistics_toPipe[d][f]);
		}
	}
	return temp;
}
