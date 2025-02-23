#include <stdio.h>
#include <stdlib.h>

#define NO_ERROR 1
#define ERROR -1

typedef struct listNode listNode;

typedef struct listNode{
	
	void * data;
	listNode * nextNode;
		
}listNode;

typedef struct List{

	listNode * firstNode;
	listNode * lastNode;
	int numOfNodes;

}List;

/*----------- List utilities --------------*/

// constructors
List * createList();
listNode * create_listNode(void * data);

// insertion functions
int insert_toList(List * list,void * data);

// geters
void * getData_fromList(List * list,int index);
int get_numOfNodes(List * list);

// search functions
void * findValue(List * list,void * value,int (*comparator)(void*,void*));

// destructors
int deleteList(List * list);

// print function
void printList(List * list,void (*printData)(void*));
