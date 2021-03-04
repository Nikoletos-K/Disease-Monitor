#include "patientData.h"

/* ---  Signal flags --- */
volatile sig_atomic_t SIGUSR1_flag;
volatile sig_atomic_t SIGINT_flag;
volatile sig_atomic_t SIGCHILD_flag;
volatile sig_atomic_t sigusr1_pid;
volatile sig_atomic_t sigchld_pid;
int SUCCESS;
int FAIL;

int main(int argc,char **argv){

	/* --- Blocking signals set initialazation--- */
	sigset_t blocksignals;
	sigfillset(&blocksignals);
	sigprocmask(SIG_SETMASK,&blocksignals,NULL);

	int index=1,b_index;
	int numOfWorkers=0,bufferSize=0;
	pid_t pid;
	DIR * input_dir;
	struct dirent * direntp;
	char cwd[PATH_MAX] = "./";
	char port[50],serverIP[50];
	port[0] = '\0',serverIP[0] = '\0';

	/* Signals sets and handlers initialazation */
	static struct sigaction actSIGUSR1,actSIGINT_OR_SIGQUIT,actSIGCHILD;
	actSIGUSR1.sa_sigaction = catchSIGUSR1andPid;
	actSIGINT_OR_SIGQUIT.sa_handler = catchSIGINT_OR_SIGQUIT;
	actSIGCHILD.sa_sigaction = catchSIGCHILD;

	sigfillset(&(actSIGUSR1.sa_mask));
	sigfillset(&(actSIGINT_OR_SIGQUIT.sa_mask));
	sigfillset(&(actSIGCHILD.sa_mask));

	sigaction(SIGUSR1,&actSIGUSR1,NULL);
	sigaction(SIGINT,&actSIGINT_OR_SIGQUIT,NULL);
	sigaction(SIGQUIT,&actSIGINT_OR_SIGQUIT,NULL);
	sigaction(SIGCHLD,&actSIGCHILD,NULL);

	/*--- reading command-line arguments ---*/
	while(index<argc){
		switch(input(argv[index])){
			case w:
				numOfWorkers = atoi(argv[index+1]);
				break;
			case b:
				b_index = index+1;
				bufferSize = atoi(argv[index+1]);
				break;
			case i:
				input_dir = opendir(argv[index+1]);
				strcat(cwd,argv[index+1]);
				if(input_dir==NULL)
					printf("ERROR: Directory %s didn't open\n",argv[index+1]);
				break;
			case p:
				/* case serverPort */
				strcpy(port,argv[index+1]);
				break;
			case s:
				/* case serverIP */
				strcpy(serverIP,argv[index+1]);
				break;
			case inputError:
				errorHandler(inputError,NULL);
				break;
		}
		index=index+2;
	}

	if(numOfWorkers<=0 || bufferSize<=0)	// Checking if arguments are valid
		errorHandler(commandLine,NULL);

	/* ------------ Reading and storing input directories subdirs ----------*/
	int numOfSubDirs = 0;
	char **subdirsArray = malloc(sizeof(char *));
	while((direntp=readdir(input_dir))!=NULL){
		if(strcmp(direntp->d_name,".") && strcmp(direntp->d_name,"..")){
			char pathbuffer[PATH_MAX];
			getcwd(pathbuffer,PATH_MAX);
			strcat(pathbuffer,cwd+1);
			strcat(pathbuffer,"/");
			strcat(pathbuffer,direntp->d_name);
			struct stat mydir;
			if(stat(pathbuffer,&mydir)!=0)
				perror("Stat failed");
			if(S_ISDIR(mydir.st_mode)){		// Check if directories data are directories and not files
				char path[PATH_MAX];
				strcpy(path,cwd);
				strcat(path,"/");
				strcat(path,direntp->d_name);

				if(numOfSubDirs!=0)
					subdirsArray = realloc(subdirsArray,(numOfSubDirs+1)*sizeof(char*));
				subdirsArray[numOfSubDirs] = strdup(path);
				numOfSubDirs+=1;
			}
		}
	}

	/* ------ Cheking if subdirs are less than workers ----------*/
	
	if(numOfWorkers>numOfSubDirs){
		numOfWorkers = numOfSubDirs;
		printf("Number of workers is geater than number of subdirectories,so %d Workers will be created \n",numOfWorkers);
	}

	npipe_connection **connectionsArray;
	connectionsArray = malloc(numOfWorkers*sizeof(npipe_connection *));

	/* ----- Creating worker processes ------ */

	fprintf(stdout,"------- WORKERS INFORMATION -------\n");
	for(int i=0;i<numOfWorkers;i++){

		connectionsArray[i] = create_pipeConnection(i);
		switch(pid = fork()){
			case -1:	
				/* if fork failed */
				perror("Fork failed");
				exit(1);
			case 0:		
				/* Child-Process (WORKER) */
				execlp("./worker","./worker","-pp",connectionsArray[i]->connection_names[PARENT],"-pc",connectionsArray[i]->connection_names[CHILD],"-b",argv[b_index],NULL);
				break;
			default:
				/* Parent-Process (SERVER) */
				connectionsArray[i]->pid = pid;
				break;
		}
	}

	/* ---- Preparing pipes for all the processes for passing the name of subdirs to workers -----*/
	
	int wfd;
	struct pollfd fdarray[numOfWorkers];

	for(int i=0;i<numOfWorkers;i++)
		fdarray[i].fd = -1;

	/* ---- Opening parent pipes -----*/

	int i=0;
	while(1){
		wfd = open(connectionsArray[i]->connection_names[PARENT],O_WRONLY);

		int j=0;
		while(j<numOfWorkers && fdarray[j].fd>2) j++;
		if(j==numOfWorkers) break;

		if(wfd<0){
			if(errno==ENXIO)
				continue;
			else
				fprintf(stderr,"Pipe %s didn't open : %s \n",connectionsArray[i]->connection_names[PARENT],strerror(errno));
		}else{
			fdarray[i].fd = wfd;
			fdarray[i].events = POLLOUT;
		}

		i++;		
		i=i%(numOfWorkers);
	}

	/* ---- Destributing subdirs to workers ----------*/

	for(int i=0;i<numOfSubDirs;i++)
		insert_subdirectory(connectionsArray[i%numOfWorkers],subdirsArray[i]);

	char * subdirs_toSent[numOfWorkers];
	int size;
	for(int i=0;i<numOfWorkers;i++){
		size = strlen(connectionsArray[i]->subdirs_concat)+1;
		connectionsArray[i]->subdirs_concat = insertSizetoMessage(connectionsArray[i]->subdirs_concat,size);
		subdirs_toSent[i] = connectionsArray[i]->subdirs_concat;
	}

	if(communicateWithWorkers(subdirs_toSent,numOfWorkers,fdarray,bufferSize,connectionsArray,NULL)!=NULL)
		fprintf(stderr,"Master communication with worker failed\n");

	
	/* ---------------- Sending ip and port to all workers -------------- */

	strcat(port,"%");
	strcat(port,serverIP);
	char * message = strdup(port);
	char * portAndIP[numOfWorkers];
	message = insertSizetoMessage(message,strlen(message)+1);

	for(int i=0;i<numOfWorkers;i++)
		portAndIP[i] = message;

	if(communicateWithWorkers(portAndIP,numOfWorkers,fdarray,bufferSize,connectionsArray,NULL)!=NULL)
		fprintf(stderr,"Master communication with worker failed\n");

	sigprocmask(SIG_UNBLOCK,&blocksignals,NULL);	// now unblock signals
	int stayAlive=1;

	/* --------- Ready to catch signals --------- */
	while(stayAlive){
		if(SIGUSR1_flag){	// case catching SIGUSR1
			sigprocmask(SIG_SETMASK,&blocksignals,NULL);
			fprintf(stdout,"Master: Catching SIGUSR1 (%ld)\n",(long)getpid());
			ReadAndPrintSummaryStatistics(numOfWorkers,fdarray,bufferSize,sigusr1_pid,connectionsArray);
			sigprocmask(SIG_UNBLOCK,&blocksignals,NULL);
			SIGUSR1_flag=0;
		}

		if(SIGINT_flag){ // case catching SIGINT
			fprintf(stdout,"Master: Catching SIGINT (%ld)\n",(long)getpid());
			stayAlive=0;
		}

		if(SIGCHILD_flag){ // case catching SIGCHLD
			sigprocmask(SIG_SETMASK,&blocksignals,NULL);
			fprintf(stdout,"Master: Catching SIGCHLD (%ld)\n",(long)getpid());
			replaceChild(sigchld_pid,numOfWorkers,connectionsArray,argv[b_index],fdarray,portAndIP);
			sigprocmask(SIG_UNBLOCK,&blocksignals,NULL);
			SIGCHILD_flag=0;
		}
	}

	for(int i=0;i<numOfWorkers;i++)		// kill all workers
		kill(connectionsArray[i]->pid,SIGKILL);

	while(wait(NULL)>0);	// wait for any left child to be killed

	/* ----  Printing to log_file ---- */ 
	char filename[50];
	sprintf(filename,"log_file.%ld",(long)getpid());
	FILE * log = fopen(filename,"w+");
	for(int i=0;i<numOfWorkers;i++){
		for(int d=0;d<(connectionsArray[i]->numOfSubdirs);d++){
			char country[50];
			getCountry(connectionsArray[i]->subdirectories[d],country);
			fprintf(log,"%s\n",country);fflush(log);		
		}
	}
	fprintf(log,"TOTAL %d\n",FAIL+SUCCESS);fflush(log);
	fprintf(log,"SUCCESS %d\n",SUCCESS);fflush(log);
	fprintf(log,"FAIL %d\n",FAIL);fflush(log);				
	fclose(log);

	/* ---- Free all allocated memory ---- */
	closedir(input_dir);
	for(int i=0;i<numOfWorkers;i++){
		destroy_pipeConnection(connectionsArray[i]);
	}
	free(message);
	for(int i=0;i<numOfSubDirs;i++)
		free(subdirsArray[i]);
	free(connectionsArray);
	free(subdirsArray);
	fprintf(stdout,"-> Master finished\n");					
return 0;
}

void catchSIGUSR1andPid(int signo,siginfo_t *info,void *data){

	sigusr1_pid = info->si_pid;
	SIGUSR1_flag = 1;
}

void catchSIGINT_OR_SIGQUIT(int signo){

	SIGINT_flag = 1;
} 

void catchSIGCHILD(int signo,siginfo_t *info,void *data){

	sigchld_pid = info->si_pid;
	SIGCHILD_flag = 1;
}