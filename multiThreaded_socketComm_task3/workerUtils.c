#include "patientData.h"

char * readfromPipe(int bufferSize,int readfd){
	/* reading data from a pipe with the protocol that has been used in all the project */

	char * pipeRead = malloc(1);
	int bytesRead,sizeofReads=0,ReadfromParent=1,index=0,packageSize = -1;
	char size[50];
	while(ReadfromParent){

		char buffer[bufferSize];
		bytesRead=read(readfd,buffer,bufferSize);
		switch(bytesRead){
			case -1:
				if(errno == EAGAIN)
					break;
				if(errno == EINTR)
					printf("STOP\n");
				break;
			case 0:	break;
			default:
				pipeRead = realloc(pipeRead,(sizeofReads+bytesRead)*sizeof(char));
				strncpy(pipeRead+sizeofReads,buffer,bytesRead);
				sizeofReads += bytesRead;
				break;
		}

		if(packageSize != -1 && sizeofReads>=packageSize+index+1)
			break;
				
		if(packageSize==-1 && bytesRead>0){

			while(index<sizeofReads && pipeRead[index]!='%' ){
				size[index] = pipeRead[index];
				index++;
			}
			if(index<sizeofReads && pipeRead[index]=='%'){
				size[index] = '\0';
				packageSize = atoi(size);

				if(sizeofReads>=packageSize+index+1)
					break;
			}
		}
	}
	return pipeRead;	
}

void rbtStatistics(RBTNode * node,int * ageCounters,int (*ageClass)(void*)){
	/* Finding for each age class number of patients */
	int class;
	if(node==GUARD)
		return;
	rbtStatistics(node->left,ageCounters,ageClass);
	class=(*ageClass)(node->data);
	ageCounters[class]++; 
	rbtStatistics(node->right,ageCounters,ageClass);
}

char * sendSummaryStatistics(HashTable * hashtable,char * toPipe){
	/* Send to parent through pipe Summary Statistics */

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
				rbtStatistics(treeRoot,ageArray,ageCondition);	// Calculating statistics per disease
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

int diseaseFrequency(char * disease,char * date1,char * date2,char * country,HashTable * hashtable){

	int numOfpatients;
	numOfpatients = findKeyData(hashtable,disease,date1,date2,dateComparator,NULL,NULL);
	return numOfpatients;
}

void FindSpecificNodes(RBTNode * node,int * counter,valueType key1,valueType key2,int (*comparator)(valueType,valueType),char * (*getExitDate)(void *)){
	/* Function that finds patients with exit date between key1,key2 */

	if(node==GUARD)
		return;

	FindSpecificNodes(node->left,counter,key1,key2,comparator,getExitDate);
	if(strcmp("--",getExitDate(get_RBTData(node))))
		if((*comparator)((void*)key1,(void*)getExitDate(get_RBTData(node)))<=0 && (*comparator)((void*)key2,(void*)getExitDate(get_RBTData(node)))>=0)
			(*counter)++;			
	FindSpecificNodes(node->right,counter,key1,key2,comparator,getExitDate);
}


char * getExitDate(void * p){
	return ((Patient*)p)->exitDate;
}

int discharges(char * disease,char * date1,char * date2,char * country,HashTable * hashtable){
	/* Finding #patients with exit date between key1,key2 for a specific disease */

	int numOfpatients;
	int loops = 0;
	int hash = hashFunction(disease,strlen(disease)+1)%(hashtable->hashtableSize);

	if(hashtable->buckets[hash]==NULL)
		return -1;

	HTRecord * tempRecord;
	Bucket * tempBucket = hashtable->buckets[hash];


	while(tempBucket!=NULL){	/* search the list of buckets */

		tempRecord = (HTRecord *) tempBucket->bucketMemory;

		while(loops<tempBucket->records){	/* search inside buckets */

			if(strcmp(disease,tempRecord->key)==0){		/* find specific hashtable key */
				numOfpatients = 0;
				RBTNode * treeRoot = (RBTNode *) getValue(tempRecord);
				FindSpecificNodes(treeRoot,&numOfpatients,date1,date2,dateComparator,getExitDate);
				return numOfpatients;	
			}

			if(loops < tempBucket->records-1){
				tempRecord++;
			}
			loops++;
		}

		tempBucket = tempBucket->next;
		loops=0;
	}

	return -1;


}

char * topk(int k,char * disease,char * date1,char * date2,HashTable * hashtable,char * result){

	Heap * heap = heapConstruct();

	int loops = 0;
	int hash = hashFunction(disease,strlen(disease)+1)%(hashtable->hashtableSize);

	if(hashtable->buckets[hash]==NULL){
		char number[100];number[0] = '\0';
		sprintf(number,"!Disease doesn't exist in this country");
		result = realloc(result,strlen(result)+strlen(number)+1);
		strcat(result,number);
		destroyHeap(heap);
		return result;
	}

	HTRecord * tempRecord;
	Bucket * tempBucket = hashtable->buckets[hash];

	int finish=0;
	int total=0;
	int numfound=0;
	while(tempBucket!=NULL){	/* search the list of buckets */

		tempRecord = (HTRecord *) tempBucket->bucketMemory;

		while(loops<tempBucket->records){	/* search inside buckets */

			if(strcmp(disease,tempRecord->key)==0){		/* find specific hashtable key */

				RBTNode * treeRoot = (RBTNode *) getValue(tempRecord);
				int ageClasses[4];
				for(int i=0;i<4;i++) ageClasses[i]=0;
				ageClassesfromTree(treeRoot,ageClasses,date1,date2,ageCondition,dateComparator);
				for(int i=0;i<4;i++){
					total += ageClasses[i];
					insert_toHeap(heap,(void *) createHeapData(i,ageClasses[i]),heapComparator);
				}	
				finish=1;
				numfound++;
				break;
			}

			if(finish) break;

			if(loops < tempBucket->records-1){
				tempRecord++;
			}
			loops++;
		}
		tempBucket = tempBucket->next;
		loops=0;
	}

	if(numfound==0){
		char number[100];number[0] = '\0';
		sprintf(number,"!Disease doesn't exist in this country");
		result = realloc(result,strlen(result)+strlen(number)+1);
		strcat(result,number);
		destroyHeap(heap);
		return result;
	}

	for(int i=0;i<k;i++){		// read heap - extract max
		heapData * hd = (heapData *) extractMax_fromHeap(heap,heapComparator);
		char number[100];number[0] = '\0';
		float percentage;
		if(total!=0)
			percentage = (((float)hd->heapKey)/((float)total))*100;
		else
			percentage=0;
		sprintf(number,"%d$%0.0f",hd->data,percentage);
		strcat(number,"%");
		result = realloc(result,strlen(result)+strlen(number)+1);
		strcat(result,number);
		free(hd);
	}
	if(k<4){		// extract and those that left in order to be deleted
		for(int i=0;i<(4-k);i++){
			heapData * hd = (heapData *) extractMax_fromHeap(heap,heapComparator);
			free(hd);
		}
	}
	return result;
}


void ageClassesfromTree(RBTNode * node,int * ageCounters,char * key1,char * key2,int (*ageClass)(void*),int (*comparator)(const void*,const void*)){
	/* Age classes between dates */
	
	if(node==GUARD)
		return;

	ageClassesfromTree(node->left,ageCounters,key1,key2,ageClass,comparator);
	if((*comparator)((void*)key1,(void*)GetKey(node))<=0 && (*comparator)((void*)key2,(void*)GetKey(node))>=0){	// else if nodes key is between these values count it
		int class=(*ageClass)(node->data);
		ageCounters[class]++; 
	}	
	ageClassesfromTree(node->right,ageCounters,key1,key2,ageClass,comparator);
	
}

