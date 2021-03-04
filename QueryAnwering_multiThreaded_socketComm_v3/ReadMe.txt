#----------------------------------- ReadMe ----------------------------------#

Name: Konstantinos Nikoletos
sdi:  1115201700104

- Compile and execution as those proposed
	
	~ make : Type 'make' for compile

- Every query and data structure has been created
- Details:
	
	1. Data structures

	 - Red-black-tree:
		I choose a Red-Black-Tree that is also balnced and quicker than a simple AVL.
		Also it needs a little bit more space.It's implemented with a generic  way
		Complexity:

					Average		Worst-case
		Search:		O(logn)		  O(logn)
		Insert: 	O(logn) 	  O(logn)
		Space: 		  O(n)			O(n)

	 - Hashtable:
		I store a counter to buckets and records ,for the quiries that don;t have a date constrain.So no read to the tree but instant read of that counter

	 - Heap: 
	 	I created a generic binary maxHeap . The path to every heap's node is found by reading current heap size as a binary,isolating the #height last bits and reading them from left to right.For every 1 it goes right and for every 0 it goes left.  

	2. Date must be compulsory to this way: dd-mm-yyyy

	3. Communication protocoll

	 - Everything in pipes is passed as string with a size in the begining and it is splited by specific chars (like $,#,etc)
	 - Buffersize can be from 1 to PIPE_MAX
	 - Nothing is passed bigger than buffersize

	4. master // diseaseAggregator.c

	 - Implemantation almost same with project 2
	 - Port is set to 0 and gets a free port  
	 - Signal handling same with project 2

	5. whoServer.c

	 - Finishes with a (SIGINT) handler ,but has some memory leaks because of forced finish of threads (SIGKILL)
	 	All allocated memory is being freed
	 - Socket and cyclic buffer implemntation is very similar with the slides
	 - If a worker terminates,I notice and inform the data structures from socket function connect() which fails if port has a problem
	 	with error in errno ECONNREFUSED
	 - numPatientAdmissions and numPatientDischarges is same with project 2
	 - Server prints both statistics and queries
	 - Clean printing in stdout is achieved by using flockfile(stdout) and funlockfile(stdout) 

	6. listen()
	 - As backlog I use SOMAXCONN

	7. whoClient.c

	 - Reads lines and creates a thread per line, when #lines read = #numThreads ,threads activated and send-recieve information
	 	with whoServer.For every numThreads I wait for them to finish and then new lines are read and new threads created
	 - Thread scheduling in order to start concurrent numThreads is based on a pseudo-code in stackoverflow

	! All queries and tests has been tested only in localhost ips and ports.