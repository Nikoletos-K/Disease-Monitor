![](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)

# Disease monitor system 2nd version
The purpose of this work is to familiarize me with creating processes using
system calls fork/exec, process communication through pipes, use of low-level I / O and creation
bash scripts.
As part of this work you I implemented a distributed information processing tool that
will receive, process, record and answer questions about viruses.
Specifically, I implemented the diseaseAggregator application which creates a series of Worker
processes that, along with the application, answers user queries.

### System architecture

![](./images/sp2.png?raw=true "System architecture")

## Compile and execution 
Type ```make``` for a simple user interface (validator accepted)

## diseaseAggregator Application

The diseaseAggregator application will be used as follows:\
```./diseaseAggregator –w numWorkers -b bufferSize -i input_dir```
where:
- The numWorkers parameter is the Worker number of processes that the application will create.
- The bufferSize parameter: is the size of the buffer for reading over pipes.
- The input_dir parameter: is a directory that contains subdirectories with the files that will
processed by Workers. Each subdirectory will have the name of a country and will contain files with
names that are dates in the DD-MM-YYYY format. For example input_dir could
contain subdirectories China / Italy / and Germany / which have the following files:
  ```
  / input_dir / China / 21-12-2019
  / input_dir / China / 22-12-2019
  / input_dir / China / 23-12-2019
  / input_dir / China / 31-12-2019
  …
  / input_dir / Italy / 31-01-2020
  / input_dir / Italy / 01-02-2020
  …
  / input_dir / Germany / 31-01-2020
  / input_dir / Germany / 20-02-2020
  / input_dir / Germany / 02-03-2020
  …
  ```


Each DD-MM-YYYY file contains a series of patient records where each line describes a
patient admitted / discharged to / from hospital that day containing recordID, name
his, the virus, and his age. For example if the contents of the file / input_dir / Germany / 02-
03-2020 of the file is:
```
889 ENTER Mary Smith COVID-2019 23
776 ENTER Larry Jones SARS-1 87
125 EXIT Jon Dupont H1N1 62
```
means that in Germany, on 02-03-2020, two patients (Mary Smith and Larry) were hospitalized
Jones) with cases of COVID-2019 and SARS-1 while Jon Dupont with H1N1 was discharged from hospital. For
an EXIT record must already have an ENTER record in the system with the same recordID and
earlier date of importation. Otherwise the word ERROR is printed on a blank
line (see also the description of Workers below).


Specifically, an entry is an ASCII line of text that consists of the following elements:
1. recordID: a string (it can have a single digit or letter) that in a unique way
determines each such record.
2. The string ENTER or EXIT: indicates admission to or discharge from a hospital.
3. patientFirstName: a string consisting of letters without spaces.
4. patientLastName: a string consisting of letters without spaces.
5. disease: a string consisting of letters, numbers, and possibly a
hyphen “-“ but without gaps.
6. age: a positive (> 0), integer <= 120.
 To get started, the diseaseAggregator application should start numWorkers Workers child
processes and
 distribute subdirectories evenly with countries in input_dir to Workers.
It will start the Workers and should inform each Worker via named pipe about the subdirectories
to be undertaken by the Worker.

When the application (the parent process) finishes creating the Worker processes, it will wait
user input (commands) from the keyboard (see commands below).
Each Worker process, for each directory assigned to it, will read all its files in chronological order
row based on file names and will fill in a number of data structures that it will use for
to answer questions posed by the parent process. The choice of the number of named pipes as well
also data structures are your design choice. (You may find some useful
structures from your first job). If while reading files, Worker detects a problem
registration (eg in syntax or an EXIT record that does not have a previous ENTER record, etc.)
prints ERROR on a new line. When it finishes reading a file, Worker will send, via
named pipe, in the parent process, summary statistics of the file containing the following information:
for each virus, in that country, it will send the number of cases per age category. For
for example, for a file / input_dir / China / 22-12-2020 containing records for SARS and COVID-19 would
sends summary statistics such as:
```
22-12-2020
China
SARS
Age range 0-20 years: 10 cases
Age range 21-40 years: 23 cases
Age range 41-60 years: 34 cases
Age range 60+ years: 45 cases

COVID-19
Age range 0-20 years: 20 cases
Age range 21-40 years: 230 cases
Age range 41-60 years: 340 cases
Age range 60+ years: 450 cases
```
These statistics should be sent for each input file (ie for each date file)
by a Worker. When Worker finishes reading his files and has sent all the summaries
statistics, notifies the parent process via named pipe that Worker is ready to accept requests.


If a Worker receives a SIGINT or SIGQUIT signal then it prints to a file named log_file.xxx
(where xxx is its process ID) the name of the countries (subdirectories) it manages, the total
number of requests received by the parent process and the total number of requests answered with
success (ie there was no error in the query).


If a Worker receives a SIGUSR1 signal, it means that 1 or more new files have been uploaded to
some of the subdirectories assigned to him. Will check subdirectories to find new files,
will read them and update the data structures it holds in memory. For each new file, it will send
summary statistics in the parent process.


If a Worker process ends abruptly, the parent process will have to fork a new Worker process
will replace it. Therefore, the parent process should handle the SIGCHLD signal as well
and SIGINT and SIGQUIT.


If the parent process receives SIGINT or SIGQUIT, it must first finish editing the current
command by the user and after responding to the user, will send a SIGKILL signal to the Workers, will
is waiting for them to terminate, and will eventually print to a file named log_file.xxxx where xxx
is its process ID, the name of all the countries (subdirectories) that "participated" in the application with
data, the total number of requests received by the user and successfully answered and
total number of requests where an error occurred and / or were not completed.

The user will be able to give the following commands to the application:
- ```/listCountries```\
The application will print each country along with the process ID of the Worker process that manages the files
of. This command is useful when the user wants to add new files to a country for editing
and must know what the Worker process is to send him a notification via SIGUSR1 signal.
Output format: one line for each country and process ID.
- ```/diseaseFrequency virusName date1 date2 [country]```\
If no country argument is given, the application will print the virusName number for the disease
cases recorded in the system during [date1 ... date2]. If given
country argument, the application will print for virusName disease, the number of cases in
country recorded in space [date1 ... date2]. The date1 date2
arguments will be in DD-MM-YYYY format.
 Output format: a line with the number of cases. Example:
 153
- ```/topk-AgeRanges k country disease date1 date2```\
The application will print, for the country and the virus the top k age categories that
cases of the virus in that country and the incidence rate
their. The date1 date2 arguments will be in DD-MM-YYYY format.
- ```/searchPatientRecord recordID``` \
The parent process forwards to all Workers the request through named pipes and waits for a response from
the Worker with the record recordID. When he receives it, he prints it.
     - Example for someone who went to the hospital 23-02-2020 and left 28-02-2020:\
      ``` 776 Larry Jones SARS-1 87 23-02-2020 28-02-2020 ``` 
     - Example for someone who was admitted to the hospital on 23-02-2020 and has not yet been released:\
       ``` 776 Larry Jones SARS-1 87 23-02-2020 - ```
- ```/numPatientAdmissions disease date1 date2 [country]```\
If a country argument is given, the application will print, on a new line, the total number of patients that
entered the hospital with the disease in that country within [date1
date2]. If no country argument is given, the application will print, for each country, the number of patients with
the disease they entered the hospital in space [date1 date2]. The date1 date2
arguments will be in DD-MM-YYYY format.
- ```/numPatientDischarges disease date1 date2 [country]```\
If the country argument is given, the application will print on a new line, the total number of patients with
the disease that have been discharged from the hospital in that country within [date1
date2]. If no country argument is given, the application will print, for each country, the number of patients with
the disease that have been discharged from the hospital in space [date1 date2]. The date1 date2
arguments will be in DD-MM-YYYY format.
- ```/exit```\
Exit the application. The parent process sends a SIGKILL signal to the Workers, waiting for them to
terminate, and print to a file named log_file.xxxx where xxx is its process ID, name
of the countries (subdirectories) that “participated” in the implementation, the total number of requests that
accepted by the user and successfully answered the total number of requests where one arose
error and / or not completed. Before it terminates, it will free up all the free memory properly.


### Pipe communication
- Two named pipes per parent-child connection (1 for parent write and 1 for child)
- Opened in each child with O_NONBLOCK wherever it supported
- Using poll() for finding which pipe has data to read

### Communication protocoll
- Everything in pipes is passed as string with a size in the begining and it is splited by specific chars (like $,#,etc)
- Buffersize can be from 1 to PIPE_MAX
- Nothing is passed bigger than buffersize

### User requests

- diseaseAggregator only makes the connection between workers and user
- Every request (except /listCountries) is passed to workers and diseaseAggregator collects the results and prints them
- There are no data structures to diseaseAggregator ,as it only prints
- Summary statistics are first calculated in a worker and the start to send to parent

### Signals

- Iplemented as sets
- Handlers only change global volatile sigatomic_t flags
- When a worker catches a SIGUSR1 ,he sends also a SIGUSR1 to parent to find and print new Statistics
- When informing data structures or writing-reading from pipes all signals are blocked with sigprocmask



# Script create_infiles.sh
Bash script that creates test subdirectories and input files that you will use for
to debug your program. Of course during the development of your program
you can use small files to debug. The script create_infiles.sh works
as follows:
```./create_infiles.sh diseasesFile countriesFile input_dir numFilesPerDirectory numRecordsPerFile```
- diseaseFile: a file with virus names (one per line)
- countriesFile: a file with country names (one per line)
- input_dir: the name of a directory where the subdirectories and input files will be placed


The script does the following:
1. Checks for input numbers
2. Reads the diseasesFile and countriesFile files
3. Creates a directory with a name given in the third input_dir argument
4. Inside the directory creates subdirectories, one for each country name in
countriesFile
5. In each subdirectory, it creates numFilesPerDirectory files with a date name
DD-MM-YYYY. In each file, place numRecordsPerFile following their format
input files described above. For diseases, the script will be randomly selected by one of
the diseasesFile. For the first and last name you can create random
strings with random length from 3 to 12 characters.

