#----------------------------------- ReadMe ----------------------------------#

Name: Konstantinos Nikoletos
sdi:  1115201700104

- Compile and execution as those proposed
	
	~ make : Type 'make' for a simple user interface (validator accepted)
	~ make with_ui  : Type 'make with_ui' for the original project interface

- Every query and data structure has been created
- Details:
	
	1. AVL-tree

		I choose a Red-Black-Tree that is also balnced and quicker than a simple AVL.
		Also it needs a little bit more space.It's implemented with a generic  way
		Complexity:

					Average		Worst-case
		Search:		O(logn)		  O(logn)
		Insert: 	O(logn) 	  O(logn)
		Space: 		  O(n)			O(n)


	2. Date insertion and queiries must be compulsory to this way: dd-mm-yyyy

	3. List

		~ Insertion at the end O(1)
		~ Search function created but not used
		~ Generic

	4. Excess data structures: 

		I used a Red Black Tree with key the identities of patients. This way , when I search for an id I don't search to the list (O(n)) but I look to the RBT O(logn) .

	5. Hashtable : 

		I store a counter to buckets and records ,for the quiries that don;t have a date constrain.So no read to the tree but instant read of that counter

	6. Heap: 

		I created a generic binary maxHeap . The path to every heap's node is found by reading current heap size as a binary,isolating the #height last bits and reading them from left to right.For every 1 it goes right and for every 0 it goes left.     

	7. 
		/recordPatientExit : if a patient has already an exit date , system prints an error // validator takes it as an error // I left the command and new-result of 4 .cmd into the directory.

	* The links between generic structures and the health-care-system is pointer to functions.