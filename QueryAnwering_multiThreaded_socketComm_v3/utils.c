#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int input(char * inputStr){

	if(!strcmp(inputStr,"-w"))
		return w;
	else if(!strcmp(inputStr,"-b"))
		return b;
	else if(!strcmp(inputStr,"-i"))
		return i;
	else if(!strcmp(inputStr,"-p"))
		return p;
	else if(!strcmp(inputStr,"-s"))
		return s;
	else
		return inputError;
}

int Prompt(char * inputStr){

	if(inputStr==NULL)
		return NO_INPUT;
	
	if(!strcmp(inputStr,"/listCountries") || !strcmp(inputStr,"1"))
		return LIST;
	else if(!strcmp(inputStr,"/diseaseFrequency") || !strcmp(inputStr,"2"))
		return DISEASE_FREQUENCY;
	else if(!strcmp(inputStr,"/topk-AgeRanges") || !strcmp(inputStr,"3"))
		return TOPK_AGE_RANGES;
	else if(!strcmp(inputStr,"/searchPatientRecord") || !strcmp(inputStr,"5"))
		return SEARCH_PATIENT;
	else if(!strcmp(inputStr,"/numPatientDischarges") || !strcmp(inputStr,"6"))
		return NUM_PDISCHARGES;
	else if(!strcmp(inputStr,"/numPatientAdmissions") || !strcmp(inputStr,"7"))
		return NUM_PADMISSIONS;
	else if(!strcmp(inputStr,"/exit") || !strcmp(inputStr,"x"))
		return EXIT;
	else
		return ERROR;
}

int errorHandler(int errorCode,char * errorStr){


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
	return noError;
}

int dateCompare(const void * date1,const void * date2){		// compares two dates 

	char * d1 = *(char**) date1;
	char * d2 = *(char**) date2;

	if(!strcmp(date2,"-"))
		return 0;

	char *token;
	int d[2],m[2],y[2];

	for(int i=0;i<2;i++){
		int t=0;
		char buffer[20];
		if(i==0) strcpy(buffer,d1);
		else     strcpy(buffer,d2);

		token = strtok(buffer,"-");
		while(token!=NULL){
			if(t==0)		d[i] = atoi(token);
			else if(t==1)	m[i] = atoi(token);
			else if(t==2)	y[i] = atoi(token);
			token = strtok(NULL,"-");
			t++;
		}
	}

	if(y[0]>y[1])
		return 1;
	else if(y[0]<y[1])
		return -1;
	else{
		if(m[0]>m[1])
			return 1;
		else if(m[0]<m[1])
			return -1;
		else{
			if(d[0]>d[1])
				return 1;
			else if(d[0]<d[1])
				return -1;
			else
				return 0;
		}
	}
}

int dateExists(char * date1){	// checks if a date exists logically

	if(!strcmp(date1,"--"))
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

int dateComparator(const  void * date1,const void * date2){

	char * d1 = (char*) date1;
	char * d2 = (char*) date2;

	if(!strcmp(date2,"--"))
		return 0;

	char *token;
	int d[2],m[2],y[2];

	for(int i=0;i<2;i++){
		int t=0;
		char buffer[20];
		if(i==0) strcpy(buffer,d1);
		else     strcpy(buffer,d2);

		token = strtok(buffer,"-");
		while(token!=NULL){
			if(t==0)		d[i] = atoi(token);
			else if(t==1)	m[i] = atoi(token);
			else if(t==2)	y[i] = atoi(token);
			token = strtok(NULL,"-");
			t++;
		}
	}

	if(y[0]>y[1])
		return 1;
	else if(y[0]<y[1])
		return -1;
	else{
		if(m[0]>m[1])
			return 1;
		else if(m[0]<m[1])
			return -1;
		else{
			if(d[0]>d[1])
				return 1;
			else if(d[0]<d[1])
				return -1;
			else
				return 0;
		}
	}
}

int integerComparator(const void* v1,const void* v2){	return *((int*) v1) - *((int*) v2);  }

int stringComparator(const void * v1,const void * v2){	return strcmp((char*)v1,(char*)v2);	}

void printOptions(){

	printf("System utilities:\n");
	printf("1./listCountries\n");
	printf("2./diseaseFrequency\n");
	printf("3./topk-AgeRanges\n");
	printf("4./searchPatientRecord\n");
	printf("5./numPatientAdmissions\n");
	printf("6./numPatientDischarges\n");
	printf("7./exit\n");
}

npipe_connection * create_pipeConnection(int index){

	char fifoname[20] = "NAMED_PIPE|";
	char process_index[20];
	sprintf(process_index,"%d",index);
	strcat(fifoname,process_index);

	npipe_connection * new = malloc(sizeof(npipe_connection));
	new->connection_names = malloc(2*sizeof(char *));
	new->index = index;
	new->pipe_filedesc = malloc(2*sizeof(int));
	new->subdirectories = malloc(sizeof(char*));
	new->subdirs_concat = NULL;
	new->numOfSubdirs = 0;
	
	for(int i=0;i<2;i++){

		char buffer[20];
		strcpy(buffer,fifoname);

		if(i==PARENT){
			strcat(buffer,"|PARENT");
			new->connection_names[PARENT] = strdup(buffer);
		}else{
			strcat(buffer,"|CHILD");
			new->connection_names[CHILD] = strdup(buffer);
		}


		if(mkfifo(buffer,0666)==-1 && (errno!=EEXIST)){
			fprintf(stderr,"Named pipe %s creation failed\n",buffer);
			exit(1);
		}
	}
	return new;
}

void set_pid(pid_t pid,npipe_connection * pipe){
	pipe->pid = pid;
}

void destroy_pipeConnection(npipe_connection * pipe){
	for(int i=0;i<2;i++){
		remove(pipe->connection_names[i]);
		free(pipe->connection_names[i]);
	}
	free(pipe->subdirectories);
	free(pipe->subdirs_concat);
	free(pipe->connection_names);
	free(pipe->pipe_filedesc);
	free(pipe);
}

void insert_subdirectory(npipe_connection * pipe,char *subdirectory){
	pipe->subdirectories = realloc(pipe->subdirectories,(pipe->numOfSubdirs+1)*sizeof(char*));
	pipe->subdirectories[pipe->numOfSubdirs] = subdirectory;
	pipe->numOfSubdirs++;

	if(pipe->subdirs_concat==NULL){
		pipe->subdirs_concat = malloc((strlen(subdirectory)+2)*sizeof(char));
		strcpy(pipe->subdirs_concat,subdirectory);
		strcat(pipe->subdirs_concat,"%");
	}else{

		pipe->subdirs_concat = realloc(pipe->subdirs_concat,(strlen(pipe->subdirs_concat)+strlen(subdirectory)+2)*sizeof(char));			
		strcat(pipe->subdirs_concat,subdirectory);
		strcat(pipe->subdirs_concat,"%");

	}
}

int onlyLettersandNumbers(char * str){

	int i=0;
	while(str[i]!='\0'){
		if(str[i]>='0' && str[i]<='9')	i++;
		else if(str[i]>='a' && str[i]<='z') i++;
		else if(str[i]>='A' && str[i]<='Z') i++;
		else 
			return 0;
	}
	return 1;
}

char * insertSizetoMessage(char * str,int size){

	char tempBuffer[100];
	sprintf(tempBuffer,"%d",size);
	strcat(tempBuffer,"%");
	size = strlen(str)+strlen(tempBuffer)+1;
	str = realloc(str,(strlen(str)+strlen(tempBuffer)+1)*sizeof(char));
	char * temp = strdup(str);
	str[0] = '\0';
	strcpy(str,tempBuffer);
	strcat(str,temp);
	free(temp);
	return str;
}

void printInteger(void * i){
	printf("%d\n",*(int*)i);
}

void printString(void * i){
	printf("%s\n",(char*)i);
}

void getCountry(char * path,char * country){

	char buffer[100];buffer[0] = '\0';
	strcpy(buffer,path);
	char * token;
	token = strtok(buffer,"/");
	while(token!=NULL){
		strcpy(country,token);
		token = strtok(NULL,"/");
		if(token==NULL) break;
		country[0] = '\0';
	}
}

void insertDelimeters(char * str,char delim){
	for(int i=0;i<strlen(str);i++)
		if(str[i] == ' ')
			str[i] = delim;
}