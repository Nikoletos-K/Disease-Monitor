# In order to execute this "Makefile" just type "make"
OBJS 	= diseaseAggregator.o utils.o list.o RBTree.o HashTable.o patientFunctions.o maxHeap.o worker.o workerUtils.o diseaseAggregatorUtils.o
SOURCE	= diseaseAggregator.c utils.c list.c RBTree.c HashTable.c patientFunctions.c maxHeap.c worker.c workerUtils.c diseaseAggregatorUtils.c
HEADER  = diseaseAggregator.h list.h RBTree.h HashTable.h patientData.h maxHeap.h
OBJS_WUI= RBTree.o
OUT  	= diseaseAggregator
CC	= gcc
FLAGS =-g -c
# -g option enables debugging mode 
# -c flag generates object code for separate files

all: diseaseAggregator worker
# all: $(OBJS) 
# 	$(CC) -g $(OBJS) -o $(OUT) -lm

# compile the version with prints and ui
# with_ui: $(OBJS_WUI)
# 	$(CC) $(FLAGS) prompt.c -DWITH_UI
# 	$(CC) $(FLAGS) patientFunctions.c -DWITH_UI
# 	$(CC) $(FLAGS) utils.c -DWITH_UI
# 	$(CC) $(FLAGS) list.c -DWITH_UI 
# 	$(CC) $(FLAGS) HashTable.c -DWITH_UI
# 	$(CC) $(FLAGS) maxHeap.c -DWITH_UI
# 	$(CC) -g $(OBJS) -o $(OUT) -lm
	
diseaseAggregator: diseaseAggregator.o utils.o patientFunctions.o diseaseAggregatorUtils.o
	$(CC) -g diseaseAggregator.o utils.o patientFunctions.o diseaseAggregatorUtils.o -o diseaseAggregator

worker: worker.o utils.o patientFunctions.o RBTree.o HashTable.o workerUtils.o
	$(CC) -g worker.o utils.o patientFunctions.o RBTree.o HashTable.o workerUtils.o -o worker

# create/compile the individual files >>separately<< 
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

list.o: list.c
	$(CC) $(FLAGS) list.c

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
