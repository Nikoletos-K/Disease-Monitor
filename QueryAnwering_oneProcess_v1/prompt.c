#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "patientData.h"

int main(int argc,char **argv){

	int index=1,disease_HTEntries=0,country_HTEntries=0,bucketSize=0,numOfPatients=0;
	FILE * sourceFile;
	char id[10],firstName[NAME_BUFFER],lastName[NAME_BUFFER],diseaseId[20],country[NAME_BUFFER],entryDate[DATE_BUFFER],exitDate[DATE_BUFFER],*fileName;
	int programRunning=True;
	char buffer[30];
	int ch;

	/* ---------------------- reading command-line arguments ---------------------*/
	while(index<argc){
		switch(input(argv[index])){
			case p:
				fileName = strdup(argv[index+1]);
				sourceFile = fopen(argv[index+1],"r");
				break;
			case h1:
				disease_HTEntries = atoi(argv[index+1]);
				break;
			case h2:
				country_HTEntries = atoi(argv[index+1]);
				break;
			case b:
				bucketSize = atoi(argv[index+1]);
				break;
			case inputError:
				errorHandler(inputError,NULL);
				break;
		}
		index=index+2;
	}

	if(disease_HTEntries<=0 || country_HTEntries<=0 || bucketSize<=0)	// Checking if arguments are valid
		errorHandler(commandLine,NULL);
	

	/*--------------- Creating data structures ---------------------*/

	#ifdef WITH_UI
	printf("\n________ Data Structures creation __________\n\n");
	#endif

	initializeDataStructures();
	List * list = createList();
	RBTNode * idtreeRoot = RBTConstruct();
	HashTable * diseaseHashTable = create_HashTable(disease_HTEntries,bucketSize);
	HashTable * countryHashTable = create_HashTable(country_HTEntries,bucketSize);

	if(diseaseHashTable==NULL || countryHashTable==NULL){
		programRunning = False;
		printf("\n - Programm ends - \n\n");
		exit(1);
	}
	/*--------------- Reading from source file ---------------------*/
	#ifdef WITH_UI

	printf("\n__________ Reading Source File _____________\n\n");
	printf("->Starting reading file %s\n",fileName);
	printf("ERRORS: \n");

	#endif

	int readerror=0;
	while(!feof(sourceFile) && programRunning == True )
		readerror = insertPatient(diseaseHashTable,countryHashTable,list,&idtreeRoot,sourceFile);
	
	if(readerror==sameRecord)		// if at reading file ,occur a duplicate -> end program 
		programRunning = False;

	#ifdef WITH_UI

	if(readerror==NO_ERROR)
		printf("\tNone\n");

	#endif

	/*--------------- Prompt -------------------*/
	#ifdef WITH_UI
		if(readerror!=sameRecord && programRunning == True ){
			printf("<-End of reading file %s \n",fileName);
			printf("\n\n_________________ HEALTH CARE SYSTEM _________________\n\n");
			printOptions();		
		}else
			printf("\n - Programm ends - \n\n");
	#endif

	while(programRunning){

		printf("\n> ");
		fscanf(stdin,"%s",buffer);
		char date1[DATE_BUFFER],date2[DATE_BUFFER];

		switch(Prompt(buffer))
		{
			case GLOBAL_DSTATS:
			{
				while((ch=fgetc(stdin)) == ' ' || ch=='\t' );

				if(ch == '\n')
				
					globalDiseaseStats(diseaseHashTable,NULL,NULL);  
				
				else{
					
					ungetc(ch,stdin);
					fscanf(stdin,"%s",date1);
					
					while((ch=fgetc(stdin)) == ' ' || ch=='\t' );
					
					if(ch>='0' && ch<='9'){

						ungetc(ch,stdin);
						fscanf(stdin,"%s",date2);
						if(dateCompare(date1,date2)>0 || !dateExists(date1) || !dateExists(date2))
							#ifdef WITH_UI
								printf("\n\tERROR: The period you asked is impossible (%s,%s)\n",date1,date2);
							#else
								printf("error\n");
							#endif

						else
							globalDiseaseStats(diseaseHashTable,date1,date2);

					}else
						errorHandler(dateMissing,NULL);
				}
				break;
			}
			case DISEASE_FREQUENCY:
			{	
				char virusName[20],country[20];
				int withCountry=False;
				fscanf(stdin,"%s %s %s",virusName,date1,date2);
				while((ch=fgetc(stdin)) == ' ' || ch=='\t' );

				if(ch == '\n'){
					ungetc(ch,stdin);
				}else{
					ungetc(ch,stdin);
					fscanf(stdin,"%s",country);
					withCountry = True;
				}

				if(dateCompare(date1,date2)>0 || !dateExists(date1) || !dateExists(date2)){
					#ifdef WITH_UI
						printf("\n\tERROR: The period you asked is impossible (%s,%s)\n",date1,date2);
					#else
						printf("error\n");
					#endif
					break;
				}

				if(!withCountry)
					diseaseFrequency(diseaseHashTable,virusName,NULL,date1,date2);
				else
					diseaseFrequency(diseaseHashTable,virusName,country,date1,date2);

				break;				
			}
			case TOPK_DISEASES:
			{		
				int k;
				char country[20];
				fscanf(stdin,"%d %s",&k,country);

				while((ch=fgetc(stdin)) == ' ' || ch=='\t' );

				if(ch == '\n'){
					topK(k,diseaseHashTable,country,NULL,NULL,DISEASE);
				}else{
					
					ungetc(ch,stdin);
					fscanf(stdin,"%s",date1);
					
					while((ch=fgetc(stdin)) == ' ' || ch=='\t' );
					
					if(ch>='0' && ch<='9'){

						ungetc(ch,stdin);
						fscanf(stdin,"%s",date2);
		
						if(dateCompare(date1,date2)>0 || !dateExists(date1) || !dateExists(date2)){
							#ifdef WITH_UI					
								printf("\n\tERROR: The period you asked is impossible (%s,%s)\n",date1,date2);
							#else
								printf("error\n");
							#endif
							break;
						}

						topK(k,diseaseHashTable,country,date1,date2,DISEASE);

					}else
						errorHandler(dateMissing,NULL);
				}
				break;
			}
			case TOPK_COUNTRIES:
			{		
				int k;
				char disease[20];
				fscanf(stdin,"%d %s",&k,disease);

				while((ch=fgetc(stdin)) == ' ' || ch=='\t' );

				if(ch == '\n'){
					topK(k,countryHashTable,disease,NULL,NULL,COUNTRY);
				}else{
					
					ungetc(ch,stdin);
					fscanf(stdin,"%s",date1);
					
					while((ch=fgetc(stdin)) == ' ' || ch=='\t' );
					
					if(ch>='0' && ch<='9'){

						ungetc(ch,stdin);
						fscanf(stdin,"%s",date2);

						if(dateCompare(date1,date2)>0 || !dateExists(date1) || !dateExists(date2)){
							#ifdef WITH_UI
								printf("\n\tERROR: The period you asked is impossible (%s,%s)\n",date1,date2);
							#else	
								printf("error\n");
							#endif
							break;
						}

						topK(k,countryHashTable,disease,date1,date2,COUNTRY);

					}else
						errorHandler(dateMissing,NULL);
				}
				break;
			}
			case INSERT_PATIENT:

				if(insertPatient(diseaseHashTable,countryHashTable,list,&idtreeRoot,NULL)==sameRecord){
					programRunning = False;
					#ifdef WITH_UI
						printf("\n - Programm ends / Duplicate found - \n\n");
					#else
						printf("error\n");
					#endif
				}
				break;
				
			case REC_PATIENT_EXIT:
			{
				char id[20],exitDate[20];
				fscanf(stdin,"%s %s",id,exitDate);
				informPatient(idtreeRoot,id,exitDate);
				break;
			}
			case NUM_CUR_PATIENTS:
			{	
				while((ch=fgetc(stdin)) == ' ' || ch=='\t' );

				if(ch == '\n')
					numCurrentPatients(diseaseHashTable,NULL);				
				else{
					char disease[20];
					ungetc(ch,stdin);
					fscanf(stdin,"%s",disease);
					numCurrentPatients(diseaseHashTable,disease);					
				}

				break;
			}
			case EXIT:
				#ifdef WITH_UI
					printf("\n__________________PROGRAM_FINISHED______________________\n\n");
				#else 
					printf("exiting\n");
				#endif
				programRunning=False;
				break;
			case ERROR:
				#ifdef WITH_UI
					printf("\n\tERROR: Command not found\n\n");
				#else
					printf("error\n");
				#endif
				break;
			default:
				break;

		}

	}

	/*--------------- free all allocated memory - closing opened files -------------------*/

	free(fileName);
	Patient * p;
	int listSize = get_numOfNodes(list); 
	destroyHashTable(diseaseHashTable);
	destroyHashTable(countryHashTable);	
	RBTDestroyTree(idtreeRoot);
	destroyDataStructures();
	for(int i=0;i<listSize;i++){
		p = (Patient *) getData_fromList(list,i);
		destroyPatient(p);
	}
	deleteList(list);
	fclose(sourceFile);
	return 0;
}