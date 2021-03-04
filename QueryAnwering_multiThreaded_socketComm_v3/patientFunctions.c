#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "patientData.h"


Patient * createPatient(char * id,char * firstName,char * lastName ,char * disease,int age,char * entryDate,char * country){

	Patient * p = malloc(sizeof(Patient));
	p->id = strdup(id);
	p->firstName = strdup(firstName);
	p->lastName = strdup(lastName);
	p->disease = strdup(disease);
	p->age = age;
	p->country = strdup(country);
	p->entryDate = strdup(entryDate);
	p->exitDate = strdup("--");	
	return p;
}

void destroyPatient(void * patient){
	
	Patient * p = (Patient *) patient;
	free(p->id);
	free(p->firstName);
	free(p->lastName);
	free(p->disease);
	free(p->entryDate);
	free(p->exitDate);
	free(p->country);
	free(p);
}

void print_patientInfo(void * p){
	printf("\t%s %s %s %s %d %s %s\n",((Patient *) p)->id,((Patient *) p)->firstName,((Patient *) p)->lastName,((Patient *) p)->disease,((Patient *) p)->age,((Patient *) p)->entryDate,((Patient *) p)->exitDate);
}
void infromExitDate(Patient * p,char * exitDate){
	free(p->exitDate);
	p->exitDate = strdup(exitDate);
}

int ageCondition(void * p){

	Patient * patient = (Patient*) p;
	if(patient->age<=20) return 0;
	else if(patient->age>20 && patient->age<=40) return 1;
	else if(patient->age>40 && patient->age<=60) return 2;
	else
		return 3;


}

void printStatistics(char **arrayStat,int numOfWorkers){

	char * cases[4] = {"0-20","21-40","41-60","60+"};
	for(int i=0;i<numOfWorkers;i++){

		char * token;
		token = strtok(arrayStat[i],"/");

		while(token!=NULL){

			if(*token=='/'){
				printf("\n");
				token = strtok(NULL,"/");
			}

			token = strtok(NULL,"%");
			if(token==NULL) break;
			printf("%s\n",token);
			token = strtok(NULL,"%");
			printf("%s\n",token);

			while(token!=NULL && *token!='/' && (token+2)!=NULL && *(token+2)!='%'){

				token = strtok(NULL,"$");
				if(token==NULL) break;
				printf("%s\n",token);
				for(int j=0;j<4;j++){					
					if(j<3)	token = strtok(NULL,"$");
					else	token = strtok(NULL,"%");
					printf("Age range %s years: %s cases\n",cases[j],token);
				}
				printf("\n");
			}
		}
	}
}

int countryPatients(void * patient,char * country){		// function that compares patient-country with wanted country

	if(!strcmp(((Patient *) patient)->country,country))
		return True;
	else 
		return False;
}

int notexitedPatients(void * patient,char * exit){		// same with disease

	if(strcmp(((Patient *) patient)->exitDate,exit))
		return True;
	else 
		return False;
}

heapData * createHeapData(int data,int key){		// heap struct that connects my program with generic maxHeap

	heapData * hd = malloc(sizeof(heapData));
	hd->data = data;
	hd->heapKey = key;
	return hd;
}

int heapComparator(void * value1,void * value2){	// heap keys comparator - integerComparator

	return integerComparator((void*)&(((heapData *)value1)->heapKey),(void*)&((heapData *)value2)->heapKey);

}