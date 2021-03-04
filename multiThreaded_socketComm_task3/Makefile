# In order to execute this "Makefile" just type "make"
OBJS 	= diseaseAggregator.o utils.o RBTree.o HashTable.o patientFunctions.o maxHeap.o worker.o workerUtils.o diseaseAggregatorUtils.o whoServer.o whoClient.o whoServerUtils.o
SOURCE	= diseaseAggregator.c utils.c RBTree.c HashTable.c patientFunctions.c maxHeap.c worker.c workerUtils.c diseaseAggregatorUtils.c whoServer.c whoClient.c whoServerUtils.c
HEADER  = diseaseAggregator.h list.h RBTree.h HashTable.h patientData.h maxHeap.h whoServer.h
OUT  	= master worker whoServer whoClient diseaseAggregator
CC	= gcc
FLAGS =-g -c
# -g option enables debugging mode 
# -c flag generates object code for separate files

all: master worker whoServer whoClient

whoServer: whoServer.o utils.o patientFunctions.o whoServerUtils.o
	$(CC) -g whoServer.o utils.o patientFunctions.o whoServerUtils.o -o whoServer -lpthread

master: diseaseAggregator.o utils.o patientFunctions.o diseaseAggregatorUtils.o
	$(CC) -g diseaseAggregator.o utils.o patientFunctions.o diseaseAggregatorUtils.o -o master

worker: worker.o utils.o patientFunctions.o RBTree.o HashTable.o workerUtils.o maxHeap.o
	$(CC) -g worker.o utils.o patientFunctions.o RBTree.o HashTable.o workerUtils.o maxHeap.o -o worker -lm

whoClient: whoClient.o utils.o patientFunctions.o whoServerUtils.o
	$(CC) -g whoClient.o utils.o patientFunctions.o whoServerUtils.o -o whoClient -lpthread

# create/compile the individual files >>separately<<

whoClient.o: whoClient.c
	$(CC) $(FLAGS) whoClient.c 

whoServer.o: whoServer.c
	$(CC) $(FLAGS) whoServer.c 

whoServerUtils.o: whoServerUtils.c
	$(CC) $(FLAGS) whoServerUtils.c 

diseaseAggregator.o: diseaseAggregator.c
	$(CC) $(FLAGS) diseaseAggregator.c 

diseaseAggregatorUtils.o: diseaseAggregatorUtils.c
	$(CC) $(FLAGS) diseaseAggregatorUtils.c 

worker.o: worker.c
	$(CC) $(FLAGS) worker.c 

utils.o: utils.c
	$(CC) $(FLAGS) utils.c

patientFunctions.o: patientFunctions.c
	$(CC) $(FLAGS) patientFunctions.c

RBTree.o: RBTree.c
	$(CC) $(FLAGS) RBTree.c

HashTable.o: HashTable.c
	$(CC) $(FLAGS) HashTable.c

maxHeap.o: maxHeap.c
	$(CC) $(FLAGS) maxHeap.c

workerUtils.o: workerUtils.c
	$(CC) $(FLAGS) workerUtils.c

# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)
