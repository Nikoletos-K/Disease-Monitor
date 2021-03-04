#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int input(char * inputStr){

	if(!strcmp(inputStr,"-p"))
		return p;
	else if(!strcmp(inputStr,"-h1"))
		return h1;
	else if(!strcmp(inputStr,"-h2"))
		return h2;
	else if(!strcmp(inputStr,"-b"))
		return b;
	else
		return inputError;
}

int Prompt(char * inputStr){

	if(!strcmp(inputStr,"/globalDiseaseStats") || !strcmp(inputStr,"1"))
		return GLOBAL_DSTATS;
	else if(!strcmp(inputStr,"/diseaseFrequency") || !strcmp(inputStr,"2"))
		return DISEASE_FREQUENCY;
	else if(!strcmp(inputStr,"/topk-Diseases") || !strcmp(inputStr,"3"))
		return TOPK_DISEASES;
	else if(!strcmp(inputStr,"/topk-Countries") || !strcmp(inputStr,"4"))
		return TOPK_COUNTRIES;
	else if(!strcmp(inputStr,"/insertPatientRecord") || !strcmp(inputStr,"5"))
		return INSERT_PATIENT;
	else if(!strcmp(inputStr,"/recordPatientExit") || !strcmp(inputStr,"6"))
		return REC_PATIENT_EXIT;
	else if(!strcmp(inputStr,"/numCurrentPatients") || !strcmp(inputStr,"7"))
		return NUM_CUR_PATIENTS;
	else if(!strcmp(inputStr,"/exit") || !strcmp(inputStr,"x"))
		return EXIT;
	else
		return ERROR;
}

int errorHandler(int errorCode,char * errorStr){

	#ifdef WITH_UI

		char * error = "\n\tERROR:" ;

		if(errorStr!=NULL){
			printf("%s %s \n",error,errorStr);
			return noError;
		}


		switch(errorCode){

			case inputError:
				printf("%s Execution arguments doesn't correspond to the project \n",error);
				printf(" Suggested command: ./diseaseMonitor -p patientRecordsFile -h1 diseaseHashTableNumOfEntries -h2 HashTableNumOfEntries -b bucketSize \n");
				break;
			case commandLine:
				printf("%s Missing or wrong arguments in execution command \n",error);
				exit(1);
				break;			
			case dateMissing:
				printf("%s Command can not be executed with only one date\n",error);
				break;			
			case noError:
				break;
		}

	#else
		if(noError)
			return 0;
		printf("error\n");

	#endif

	return noError;
}

int dateCompare(char * date1,char * date2){		// compares two dates 

	if(!strcmp(date2,"-"))
		return 0;

	char buffer[DATE_BUFFER];
	int d1,d2,m1,m2,y1,y2;

	strcpy(buffer,date1);
	strtok(buffer,"-");
	d1 = atoi(buffer);

	strcpy(buffer,date1+3);
	strtok(buffer,"-");
	m1 = atoi(buffer);
	
	strcpy(buffer,date1+6);
	strtok(buffer,"-");
	y1 = atoi(buffer);

	strcpy(buffer,date2);
	strtok(buffer,"-");
	d2 = atoi(buffer);

	strcpy(buffer,date2+3);
	strtok(buffer,"-");
	m2 = atoi(buffer);
	
	strcpy(buffer,date2+6);
	strtok(buffer,"-");
	y2 = atoi(buffer);


	if(y1>y2)
		return 1;
	else if(y1<y2)
		return -1;
	else{
		if(m1>m2)
			return 1;
		else if(m1<m2)
			return -1;
		else{
			if(d1>d2)
				return 1;
			else if(d1<d2)
				return -1;
			else
				return 0;
		}
	}
}

int dateExists(char * date1){	// checks if a date exists logically

	if(!strcmp(date1,"-"))
		return 1;

	char buffer[DATE_BUFFER];
	int d1,m1;

	strcpy(buffer,date1);
	strtok(buffer,"-");
	d1 = atoi(buffer);

	if(d1>31 || d1<1)
		return 0;

	strcpy(buffer,date1+3);
	strtok(buffer,"-");
	m1 = atoi(buffer);
	
	if(m1>12 || m1<1)
		return 0;

	return 1;
}

int integerComparator(void* v1,void* v2){	return *((int*) v1) - *((int*) v2);  }

int stringComparator(char* v1,char* v2){	return strcmp(v1,v2);	}

void printOptions(){

	printf("System utilities:\n");
	printf("1./globalDiseaseStats\n");
	printf("2./diseaseFrequency\n");
	printf("3./topk-Diseases\n");
	printf("4./topk-Countries\n");
	printf("5./insertPatientRecord\n");
	printf("6./recordPatientExit\n");
	printf("7./numCurrentPatients\n");
	printf("8./exit\n");
}