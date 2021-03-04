![](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)

# Disease monitor system 3rd version
The aim of this work is to familiarize me with thread programming and network communication. 
As part of this work I implemented a distributed process system that will provide the same
functionality with the diseaseAggregator application of the second task. Specifically, I implemented three
programs: 
1. a master program that will create a series of Worker processes (as did the parent process
in the second task), 
2. a multi-threaded whoServer that will collect over the network summary statistics from Worker
processes and queries from clients, and 
3. a multithreaded client whoClient program that will create a lot
threads, where each thread plays the role of a client sending queries to whoServer.

### System architecture

![](./images/sp3.png?raw=true "System architecture")


## master program
The master program will be used as follows:\
```./master –w numWorkers -b bufferSize –s serverIP –p serverPort -i input_dir``` \
where:
-  ```numWorkers```  is the Worker number of processes that the application will generate.
-  ```bufferSize``` : is the size of the buffer for reading over pipes.
-  ```serverIP``` : is the IP address of the whoServer to which the Worker processes will connect to
send him summary statistics.
-  ```serverPort``` : is the port number where the whoServer listens.
-  ```input_dir``` : is a directory that contains subdirectories with the files to be processed
the Workers. As in the second task, each subdirectory will have the name of a country and will contain files with
names that are dates in the DD-MM-YYYY format. Each DD-MM-YYYY file will have exactly the same
format that he had in the second task and will contain a series of patient records where each line will describe
a patient who was admitted / discharged to / from a hospital that day and contains the recordID, his name,
the virus, and its age.


To get started, the master program starts numWorkers Workers child processes and share
uniformly the subdirectories with the countries in input_dir in Workers (as in task 2). Starts the Workers and informs each Worker via named pipe about the subdirectories that will
undertaken by the Worker. Subdirectories will be flat, that is, they will only contain files, no
subdirectories. The parent process sends via named pipe both the IP address and the port number of whoServer.
When the creation of the Worker processes is finished, the parent process will remain (ie will not end) to
forks a new Worker process in case an existing Worker suddenly terminates.


Each Worker process, for each directory assigned to it, reads all its files in chronological order
of filenames and fills in a number of data structures that it will use to respond to
questions that will be forwarded to him by whoServer. It connects to whoServer and send it the following information:
1. a port number where the Worker process will listen for questions that will be forwarded by whoServer, and 
2. the summaries statistics (same as in the second work). 

When the Worker process finishes transferring information to whoServer, will listen to the port number it has selected and wait for connections from whoServer for requests regarding
countries it manages.

## whoServer program

WhoServer will be used as follows: \
```./whoServer –q queryPortNum -s statisticsPortNum –w numThreads –b bufferSize```\
where:
- ```queryPortNum``` is a port number where whoServer will listen for query links
by the whoClient client
- ``` statisticsPortNum ``` is a port number where whoServer will listen for connections to
summary statistics by Worker processes
- ``` numThreads ```  is the number of threads that whoServer will generate to serve
incoming connections from the network. Threads should be created once at the beginning when the
whoServer.
- ``` bufferSize ``` is the size of a circular buffer to be shared between the threads that
are created by the whoServer process. The bufferSize represents the number of file / socket descriptors that
can be stored in it (eg 10, means 10 descriptors).


When whoServer starts, the original thread should create numThreads threads. The
main (main process) thread listens on the ports queryPortNum and statisticsPortNum, accepts connections
with the accept() system call and place the file/socket descriptors that
correspond to the connections in a circular buffer of size defined by bufferSize. The initial thread is not
reading from the links it receives. Simply, whenever it accepts a connection it will place the file descriptor
which accept() returns to the buffer and will continue to accept subsequent connections. The work of numThreads
are to serve the links whose corresponding file descriptors are placed in
buffer. Each of the numThreads threads wakes up when there is at least one descriptor in the buffer.
More specifically, the inital thread will listen to statisticsPortNum for links from Worker processes to
receive the summary statistics and the port number where each Worker process listens, and will listen to
queryPortNum for connections from whoClient to receive queries about cases reported in
distributed process system.


WhoServer will accept and handle the following requests from whoClient (similar to task 2):
- ```/diseaseFrequency virusName date1 date2 [country]```\
If no country argument is given, whoServer finds the number of cases for virusName
which have been recorded in the system during [date1 ... date2]. If a country argument is given, ο
whoServer will find out about the virusName disease, the number of cases in the country that have
recorded in space [date1 ... date2]. The date1 date2 arguments will take the form
DD-MM-YYYY. The communication protocol between the client and whoServer should
manages in some way the fact that [country] is optional in this query.
- ```/topk-AgeRanges k country disease date1 date2```\
WhoServer finds, for the country and the virus disease, the top k age categories that have
cases of this virus in that country and their incidence rate. The date1 date2 arguments will be in DD-MM-YYYY format.
- ```/searchPatientRecord recordID```\
WhoServer forwards the request to all Workers and waits for a response from the Worker with the record
recordID.
- ```/numPatientAdmissions disease date1 date2 [country]```
If a country argument is given, whoServer should forward the request to the workers in order to find the
total number of patients admitted to hospital with the disease in that country within
in space [date1 date2]. If no country argument is given, he will find the number of patients with the disease
disease that entered the hospital in space [date1, date2]. The date1 date2 arguments will
have DD-MM-YYYY format.
- ```/numPatientDischarges disease date1 date2 [country]```
If given the country argument, whoServer finds the total number of patients with the disease
who have been discharged from a hospital in that country within [date1, date2]. If not given
country argument, whoServer finds the number of patients with the disease who have been discharged from
hospital in space [date1, date2]. The date1, date2 arguments will be in DD-MM-YYYY format.
When whoServer accepts a query, it forwards it to the corresponding worker processes via a socket and waits
the response from the workers. The question posed in a Worker process along with the answers
whoServer receives from this Worker, prints them to stdout. WhoServer also forwards the response to
corresponding whoClient thread that made the query.


### Implementation choices of whoServer.c 
- Finishes with a (SIGINT) handler ,but has some memory leaks because of forced finish of threads (SIGKILL)
All allocated memory is being freed
- Socket and cyclic buffer implemntation is very similar with the slides
- If a worker terminates,I notice and inform the data structures from socket function connect() which fails if port has a problem
with error in errno ECONNREFUSED
- numPatientAdmissions and numPatientDischarges is same with project 2
- Server prints both statistics and queries
- Clean printing in stdout is achieved by using flockfile(stdout) and funlockfile(stdout) 

## whoClient program
The whoClient program will be used as follows: \
```./whoClient –q queryFile -w numThreads –sp servPort –sip servIP```
- ```queryFile``` is the file that contains the queries to be sent to whoServer.
- ```numThreads``` is the number of threads that whoClient will generate for sending
queries in whoServer
- ```servPort``` is the port number where the whoServer listens to which the whoClient will connect.
- ```servIP``` is the IP address of the whoServer to which the whoClient will connect.


### The function of multithreaded whoClient is as follows
The queryFile file will start and open reads line by line. On each line there will be one command that the whoServer can accept. For each
command will create a thread that will send a command (ie a line) to whoServer. Thread
will be created but will not connect immediately to whoServer. When all the threads are created, that is, we have one
thread for each command in the file, then the threads should all start together to try to connect
to whoServer and send their command. When the command is sent, each thread will print the response received
from whoServer to stdout and can quit. When all threads are terminated, the whoClient terminates as well.

### Implementation choices of whoClient.c
- Reads lines and creates a thread per line, when #lines read = #numThreads ,threads activated and send-recieve information
with whoServer.For every numThreads I wait for them to finish and then new lines are read and new threads created
- Thread scheduling in order to start concurrent numThreads is based on a pseudo-code in stackoverflow


## Communication protocoll
- Everything in pipes is passed as string with a size in the begining and it is splited by specific chars (like $,#,etc)
- Buffersize can be from 1 to PIPE_MAX
- Nothing is passed bigger than buffersize


** All queries and tests has been tested only in localhost ips and ports **













