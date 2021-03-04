# Disease monitor system 1st version

In this work I implemented a program that accepts, processes, records and answers questions about cases of viruses. Specifically I implemented a set of structures (hash
tables, linked lists, binary trees) that allow the entry and queries in a large volume of records
patientRecord type. Although the exercise data will come from files, eventually all
Records will only be stored in main memory. 

## Compile and execution 
- make : Type ```make``` for a simple user interface (validator accepted)
- make with_ui  : Type ```make with_ui``` for the original project interface

## System interface 
The application will be called diseaseMonitor and will be used as follows:
```
./diseaseMonitor -p patientRecordsFile –h1 diseaseHashtableNumOfEntries –H2 countryHashtableNumOfEntries –b bucketSize
```
where:
- The ```diseaseHashtableNumOfEntries``` parameter is the number of positions in a table
fragmentation that the application will keep tracking patient information.
- The ```countryHashtableNumOfEntries``` parameter is the number of places in a table
fragmentation that the application will hold to detect case information by country.
- The ```bucketSize``` parameter is the number of Bytes that gives the size of each bucket to
hash tables.
- The ```patientRecordsFile``` (or some other file name) is a file that contains a number of
patient records for processing. Each line of this file describes a case of a
the name of the patient, in which country, the date of admission to the hospital and the
date of discharge. For example if the contents of the file are:
```
889 Mary Smith COVID-2019 China 25-1-2019 27-1-2019
776 Larry Jones SARS-1 Italy 10-02-2003 -
125 Jon Dupont H1N1 USA 12-02-2016 15-02-2016
```
means we have three records describing three cases in three different countries
(China, Italy, USA). In the second registration, there is no discharge date (the patient remains at
hospital). Specifically, a record is an ASCII line of text that consists of the following
data:
1. ```recordID```: a string (it can have a single digit) that in a unique way
determines each such record.
2. ```patientFirstName```: a string consisting of letters without spaces.
3. ```patientLastName```: a string consisting of letters without spaces.
4. ```diseaseID``````: a string consisting of letters, numbers, and possibly a
hyphen “-” but without spaces.
5. ```country```: a string consisting of letters without spaces.
6. ```entryDate```: date of admission to the hospital. It must have the form
DD-MM-YYYY where DD expresses the day, MM the month, and YYYY the time of entry
of the patient.
7. ```exitDate```: date the patient was discharged from the hospital. It must have the form
DD-MM-YYYY where DD expresses the day, MM the month, and YYYY the time of its output
patient or hyphen (-) which means that the patient has not yet been discharged.

## System functions

- ```/globalDiseaseStats [date1 date2]``` \
The application prints for each virus, the number of cases recorded in the system. If
given date1 date2 then the application will print for each virus, the number of cases they have
recorded in the system within the time period [date1 ... date2].
If there is a definition for [date1] there should be a definition for [date2], otherwise it will
an error message is printed to the user. 
- ```/diseaseFrequency virusName [country] date1 date2```\
If no country argument is given, the application will print the virusName number for the disease
cases recorded in the system during [date1 ... date2]. If given
country argument, the application will print for virusName disease, the number of cases in
country recorded in space [date1 ... date2]. 
- ```/topk-Countries k disease [date1 date2]```\
The application will print, for the disease virus, the countries that have shown the top k of the cases
of this virus. 
- ```/topk-Diseases k country [date1 date2]```\
The application will print, for the country, the diseases that are the top k of the cases
in space [date1 ... date2] if given. If there is a definition for [date1] you should
there is also a definition for [date2], otherwise an error message will be printed to the user.
Top / topk-Countries k disease [date1 date2]
The application will print, for the virus, the countries that have shown the top k of the cases
of this virus.
- ```/insertPatientRecord recordID patientFirstName patientLastName diseaseID```\
entryDate [exitDate]
The application will introduce a new registration with its data in the system. The exitDate element is
optional.
- ```/recordPatientExit recordID exitDate```\
The application will add exitDate to the record with ID recordID.If a patient has already an exit date , system prints an error // validator takes it as an error // I left the command and new-result of 4 .cmd into the directory.
- ```/numCurrentPatients [disease]```\
If given the disease argument, the application will print the number of patients still being treated with
the disease disease. If no argument is given, the application will print the documents for each virus
patients still being treated.
- ```/exit```\
Exits the application. 

## Data structures and algorithms used
1. Red black-tree \
I choose a Red-Black-Tree that is also balnced and quicker than a simple AVL.
Also it needs a little bit more space.It's implemented with a generic  way\
Complexity:

  | - |  Average | Worst-case  |
  |---|---|---|
  | Search  | O(logn)  |  O(logn) |
  | Insert  |  O(logn) | O(logn)  |
  | Space  |  O(n) | O(n)  | 

2. List
      - Insertion at the end O(1)
      - Search function created but not used
      - Generic
3. Hashtable \
I store a counter to buckets and records ,for the quiries that don;t have a date constrain.So no read to the tree but instant read of that counter
4. Heap \
I created a generic binary maxHeap . The path to every heap's node is found by reading current heap size as a binary,isolating the #height last bits and reading them from left to right.For every 1 it goes right and for every 0 it goes left.     











