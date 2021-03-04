#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "patientData.h"


Patient * createPatient(char * id,char * firstName,char * lastName ,char * disease,char * country,char * entryDate,char * exitDate){

	Patient * p = malloc(sizeof(Patient));
	p->id = strdup(id);
	p->firstName = strdup(firstName);
	p->lastName = strdup(lastName);
	p->country = strdup(country);
	p->disease = strdup(disease);
	p->entryDate = strdup(entryDate);
	p->exitDate = strdup(exitDate);

	return p;
}

void destroyPatient(Patient * p){

	free(p->id);
	free(p->firstName);
	free(p->lastName);
	free(p->country);
	free(p->disease);
	free(p->entryDate);
	free(p->exitDate);
	free(p);
}

void print_patientInfo(void * p){
	printf("\t%s %s %s %s %s %s %s \n",((Patient *) p)->id,((Patient *) p)->firstName,((Patient *) p)->lastName,((Patient *) p)->disease,((Patient *) p)->country,((Patient *) p)->entryDate,((Patient *) p)->exitDate);
}

int globalDiseaseStats(HashTable * hashtable,char * date1,char * date2){

	#ifdef WITH_UI
		if(date1==NULL || date2==NULL)
			printf("\n    # Number of patients for every disease: \n\n");
		else
			printf("\n    # Number of patients for every disease with entry date in period %s to %s: \n\n",date1,date2);
	#endif

	numOfRecordsBetweenKeys(hashtable,date1,date2,dateCompare);
	return NO_ERROR;
}



int diseaseFrequency(HashTable * hashtable,char * virusName,char * country,char * date1,char * date2){

	if(country==NULL)
		#ifdef WITH_UI
			printf("\n\tPeriod: [%s,%s]\n\tDisease: %s\n\tRecorded patients: %d \n",date1,date2,virusName,findKeyData(hashtable,virusName,date1,date2,dateCompare,NULL,NULL));
		#else
			printf("%s %d\n",virusName,findKeyData(hashtable,virusName,date1,date2,dateCompare,NULL,NULL));
		#endif
	else
		#ifdef WITH_UI
			printf("\n\tCountry: %s \n\tPeriod: [%s,%s]\n\tDisease: %s\n\tRecorded patients: %d \n",country,date1,date2,virusName,findKeyData(hashtable,virusName,date1,date2,dateCompare,country,countryPatients));
		#else
			printf("%s %d\n",virusName,findKeyData(hashtable,virusName,date1,date2,dateCompare,country,countryPatients));
		#endif

	return NO_ERROR;	

}

int topK(int k,HashTable * hashtable,char * country,char * date1,char * date2,int diseaseORcountry){

	Heap * heap = heapConstruct();
	int loops = 0;

	if(hashtable->numOfRecords<k){		// if k is bigger than heap elements
		
		#ifdef WITH_UI		
			if(diseaseORcountry == DISEASE)
				printf("\n   - The system has stored only %d diseases so top-%d will be presented \n",hashtable->numOfRecords,hashtable->numOfRecords );
			else
				printf("\n   - The system has stored only %d countries so top-%d will be presented \n",hashtable->numOfRecords,hashtable->numOfRecords );
		#endif
		k = hashtable->numOfRecords;		// k = max heap elements
	}

	for(int i=0;i<hashtable->hashtableSize;i++){

		if(hashtable->buckets[i]==NULL)
			continue;

		HTRecord * tempRecord;
		Bucket * tempBucket = hashtable->buckets[i];

		while(tempBucket!=NULL){	/* search the list of buckets */

			tempRecord = (HTRecord *) tempBucket->bucketMemory;		// read first record of bucket

			while(loops<tempBucket->records){	/* search inside buckets */

				int counter = 0;
				if(date1==NULL && date2==NULL){		// case there's no date condition

					RBTNode * treeRoot = (RBTNode *) getValue(tempRecord);		// get tree root
					
					if(diseaseORcountry == DISEASE)
						RBTFindNodesBetweenKeys(treeRoot,&counter,NULL,NULL,country,dateCompare,countryPatients);
					else
						RBTFindNodesBetweenKeys(treeRoot,&counter,NULL,NULL,country,dateCompare,diseasePatients);


					insert_toHeap(heap,(void *) createHeapData(tempRecord->key,counter),heapComparator);

				}else{		// with date condition

					RBTNode * treeRoot = (RBTNode *) getValue(tempRecord);
					if(diseaseORcountry == DISEASE)
						RBTFindNodesBetweenKeys(treeRoot,&counter,(void *) date1,(void *) date2,country,dateCompare,countryPatients);
					else
						RBTFindNodesBetweenKeys(treeRoot,&counter,(void *) date1,(void *) date2,country,dateCompare,diseasePatients);

					insert_toHeap(heap,(void *) createHeapData(tempRecord->key,counter),heapComparator);
							
				}

				if(loops < tempBucket->records-1){		// if reading last bucket record
					tempRecord++;
				}
				loops++;
			}

			tempBucket = tempBucket->next;
			loops=0;
		}
	}
	#ifdef WITH_UI
		if(diseaseORcountry == DISEASE)
			printf("\n   ~ Top %d diseases in %s ~\n\n",k,country );
		else
			printf("\n   ~ Top %d countries in %s ~\n\n",k,country );
	#endif

	for(int i=0;i<k;i++){		// read heap - extract max
		heapData * hd = (heapData *) extractMax_fromHeap(heap,heapComparator);
		#ifdef WITH_UI
			printf("    %d. %s  : %d \n",i+1,hd->data,hd->heapKey);
		#else
			if(hd->heapKey!=0)
				printf("%s %d\n",hd->data,hd->heapKey);
		#endif
		free(hd);
	}

	if(k<hashtable->numOfRecords){		// extract and those that left in order to be deleted
		for(int i=0;i<(hashtable->numOfRecords-k);i++){
			heapData * hd = (heapData *) extractMax_fromHeap(heap,heapComparator);
			free(hd);
		}
	}
	return NO_ERROR;
}


int insertPatient(HashTable * diseaseHashtable,HashTable * countryHashtable,List * list,RBTNode ** treeRoot,FILE * sourceFile){

	int ch,error=0,inserted=False;
	char id[10],firstName[NAME_BUFFER],lastName[NAME_BUFFER],diseaseId[20],country[NAME_BUFFER],entryDate[DATE_BUFFER],exitDate[DATE_BUFFER];

	if(sourceFile!=NULL)
		fscanf(sourceFile,"%s %s %s %s %s %s %s \n",id,firstName,lastName,diseaseId,country,entryDate,exitDate);	// if readind from a file
	else{
		fscanf(stdin,"%s %s %s %s %s %s",id,firstName,lastName,diseaseId,country,entryDate);	// if inserting from command

		while((ch=fgetc(stdin)) == ' ' || ch=='\t' );

		if((ch>='0' && ch<='9') || ch=='-'){
			ungetc(ch,stdin);
			fscanf(stdin,"%s",exitDate);
		}else
			strcpy(exitDate,"-");

		#ifdef WITH_UI
			printf("\n\t%s %s %s %s %s %s %s\n",id,firstName,lastName,diseaseId,country,entryDate,exitDate);
		#endif
		inserted=True;
	}

	if(dateCompare(entryDate,exitDate)>0){		// checking dates
		#ifdef WITH_UI
			printf("\tERROR: Entry date (%s) is after the exit date (%s) !\n",entryDate,exitDate);
		#endif

		error=True;
	}

	if(!dateExists(entryDate)){		// checking if date is valid
		#ifdef WITH_UI
			printf("\tERROR: Entry date (%s) doesn't exist!\n",entryDate);
		#endif
		error=True;
	}

	if(!dateExists(exitDate)){		// same
		#ifdef WITH_UI
			printf("\tERROR: Exit date (%s) doesn't exist!\n",exitDate);
		#endif
		error=True;
	}

	if(RBTFindNode(*treeRoot,id,stringComparator)!=GUARD){		// checking if this record is in the tree
		error=True;
		#ifdef WITH_UI
			printf("\tERROR: Patient with id %s already exists in system !\n",id);
			printf("\t- Record ~ %s %s %s %s %s %s %s ~ rejected \n",id,firstName,lastName,diseaseId,country,entryDate,exitDate);
		#endif
		return sameRecord;				
	}

	if(error){
		#ifdef WITH_UI
			printf("\t- Record ~ %s %s %s %s %s %s %s ~ rejected \n",id,firstName,lastName,diseaseId,country,entryDate,exitDate);
		#else
			printf("error\n");
		#endif
		return ERROR;				
	}

	Patient * patient = createPatient(id,firstName,lastName,diseaseId,country,entryDate,exitDate);
	error = insert_toList(list,patient);
	RBTInsert(treeRoot,patient,patient->id,stringComparator);
	insert_toHashTable(patient,patient->disease,patient->entryDate,diseaseHashtable,dateCompare);
	insert_toHashTable(patient,patient->country,patient->entryDate,countryHashtable,dateCompare);

	if(inserted==True)
		#ifdef WITH_UI
			printf("\tRecord inserted successfully!\n");
		#else
			printf("Record added\n");
		#endif

	return NO_ERROR;
}

int informPatient(RBTNode * treeRoot,char * id,char * exitDate){

	RBTNode * temp = RBTFindNode(treeRoot,id,stringComparator);
	if(temp==GUARD){
		#ifdef WITH_UI
			printf("\n\tRecord with id: %s not found!\n\n",id );
		#else
			printf("Not found\n");
		#endif
		return NO_ERROR;
	}


	Patient * p = (Patient *) get_RBTData(temp);

	if(strcmp(p->exitDate,"-")!=0){
		#ifdef WITH_UI
			printf("\n\tPatient with id: %s isn't stil at hospital,exited at %s \n\tRecord not informed\n",p->id,p->exitDate);
			print_patientInfo(p);
			printf("\n");
		#else
			printf("error\n");
		#endif
		return NO_ERROR;		
	}		

	if(dateCompare(p->entryDate,exitDate)>0){
		#ifdef WITH_UI
			printf("\n\t Entry date %s is after exit date %s : Impossible \n\tRecord not informed\n\n",p->entryDate,exitDate );
			print_patientInfo(p);
			printf("\n");
		#else
			printf("error\n");
		#endif

		return NO_ERROR;
	}

	free(p->exitDate);
	p->exitDate = strdup(exitDate);
	#ifdef WITH_UI
		printf("\n\tPatients exit date informed successfully\n");
		printf("\tInformed record: \n");
		print_patientInfo(p);
		printf("\n");
	#else
		printf("Record updated\n");
	#endif

	return NO_ERROR;
}

int hospitalized(void * patient,char * null){		// function that checks if a patient is stil in hospital

	if(!strcmp(((Patient *)patient)->exitDate,"-"))
		return True;
	else 
		return False;
}

int numCurrentPatients(HashTable * hashtable,char * disease){

	if(disease==NULL){
		#ifdef WITH_UI
			printf("\n     # Number of patients stil hospitalized # \n\n");
		#endif
		printConditionHT(hashtable,hospitalized);		
	}else{
		char ch[2] = "-";
		int counter = findKeyData(hashtable,disease,NULL,NULL,dateCompare,ch,hospitalized);
		if(counter<0)
			#ifdef WITH_UI
				printf("\n\tDisease %s not stored in system\n\n",disease );
			#else
				printf("error\n");
			#endif
		else
			#ifdef WITH_UI
				printf("\n\t[%s]: Number of patients stil hospitalized %d \n",disease,counter);
			#else
				printf("%s %d\n",disease,counter);
			#endif
	}


	return NO_ERROR;
}

int countryPatients(void * patient,char * country){		// function that compares patient-country with wanted country

	if(!strcmp(((Patient *) patient)->country,country))
		return True;
	else 
		return False;
}

int diseasePatients(void * patient,char * disease){		// same with disease

	if(!strcmp(((Patient *) patient)->disease,disease))
		return True;
	else 
		return False;
}

int idComparator(void * patient,void * id){		// compares patient ids
	return strcmp((((Patient*)patient)->id),(char*)id);
}

heapData * createHeapData(char * data,int key){		// heap struct that connects my program with generic maxHeap

	heapData * hd = malloc(sizeof(heapData));
	hd->data = data;
	hd->heapKey = key;
	return hd;
}

int heapComparator(void * value1,void * value2){	// heap keys comparator - integerComparator

	return integerComparator((void*)&(((heapData *)value1)->heapKey),(void*)&((heapData *)value2)->heapKey);

}