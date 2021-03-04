#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "RBTree.h"

/*---------------------------------Create_Functions-------------------------------------------*/
void initializeDataStructures(){
	GUARD = createGuard();
}


RBTNode * createGuard(){

	RBTNode * temp = malloc(sizeof(RBTNode));
	temp->parent = temp -> left = temp -> right = GUARD;			/* Guard:GLOBAL POINTER with color istead of NULL */
	temp->data = NULL;
	temp->key = NULL;
	temp->color = BLACK ;
	return temp;

}


RBTNode * RBTConstruct(){ 
	return GUARD;
}

RBTNode * RBTnewNode(void * data,valueType key){

	RBTNode * tempNode = malloc(sizeof(RBTNode));
	tempNode->parent=GUARD;
	tempNode->right=GUARD;
	tempNode->left=GUARD;
	tempNode->data = data;
	tempNode->color = RED;
	RBTInitialiseKey(tempNode,key);

	return tempNode;	
}

void RBTInitialiseKey(RBTNode * n,char * key){

	n->key = strdup(key);
}


/*------------------------------------- Getters -------------------------------------------*/


RBTNode * GetParent(RBTNode * n) {  return n -> parent; }

RBTNode * GetGrandParent(RBTNode * n) { return GetParent(n)->parent;}

nodeColor GetColor(RBTNode * n){ return n->color ; }

char * GetKey(RBTNode * n){	return n->key; }

void SetColor(RBTNode * n,nodeColor color){ n->color=color; }

int RBTempty(RBTNode * root){ return (root == GUARD ); }

void * get_RBTData(RBTNode * node){	return node->data; }

/*----------------------Search_function---------------------------*/

RBTNode * RBTFindNode(RBTNode * node,char * key,int (*comparator)(valueType,valueType)){

	if(node==GUARD || !(*comparator)(GetKey(node),key))
		return node;

	if((*comparator)(key,GetKey(node))<0)
		RBTFindNode(node->left,key,comparator);
	else
		RBTFindNode(node->right,key,comparator);

}

void RBTPrintTreeOnCondition(RBTNode * node,void (*printData)(void * data),int (*condition)(void*,char*)){

	if(node==GUARD)
		return;

	RBTPrintTreeOnCondition(node->left,printData,condition);
	if((*condition)(node->data,NULL))
		printData(node->data);
	RBTPrintTreeOnCondition(node->right,printData,condition);
}


void RBTFindNodesBetweenKeys(RBTNode * node,int * counter,void * key1,void * key2,char * funValue,int (*comparator)(char *,char *),int (*function)(void*,char*)){
	
	if(node==GUARD)
		return;

	if(key1==NULL && key2==NULL){		// if keys are NULL

		RBTFindNodesBetweenKeys(node->left,counter,NULL,NULL,funValue,comparator,function);
		if((*function)(node->data,funValue))	// isolate and count specific tree nodes
			(*counter)++;			
		RBTFindNodesBetweenKeys(node->right,counter,NULL,NULL,funValue,comparator,function);
	
	}else{

		if((*comparator)(key1,GetKey(node))>0)	// if key1 is greater that key of node => do CUTOFF and go right
		
			RBTFindNodesBetweenKeys(node->right,counter,key1,key2,funValue,comparator,function);
		
		else if((*comparator)(key2,GetKey(node))<0)		// if key2 is less that key of node => do CUTOFF and go left
		
			RBTFindNodesBetweenKeys(node->left,counter,key1,key2,funValue,comparator,function);
		
		else if((*comparator)(key1,GetKey(node))<=0 || (*comparator)(key2,GetKey(node))<=0){	// else if nodes key is between these values count it
		
			RBTFindNodesBetweenKeys(node->left,counter,key1,key2,funValue,comparator,function);
		
			if(funValue!=NULL){		// if there's and another constrain
				if(function(node->data,funValue))
					(*counter)++;	
			}else	// if there's no other constrain count all
				(*counter)++;
			
			RBTFindNodesBetweenKeys(node->right,counter,key1,key2,funValue,comparator,function);
		}
	}
}

/*--------------------------- Rotations -----------------------------*/

void RotateLeft(RBTNode **treeRoot,RBTNode * x){

	RBTNode * y = x->right;				
	x->right = y->left;

	if(y->left != GUARD)
		y->left->parent = x;

	y->parent = GetParent(x);

	if(GetParent(x) == GUARD)
		*treeRoot = y;
	else if(x == GetParent(x)->left)
		x->parent->left = y;
	else 
		x->parent->right = y;

	y->left = x;
	x->parent = y;
}

void RotateRight(RBTNode **treeRoot,RBTNode * x){

	RBTNode * y = x->left;
	x->left = y->right;

	if(y->right != GUARD)
		y->right->parent = x;

	y->parent = GetParent(x);

	if(GetParent(x) == GUARD)
		*treeRoot = y;
	else if( x ==GetParent(x)->right )
		x->parent->right = y;
	else
		x->parent->left = y;


	y->right = x;
	x-> parent = y;
}


/*-----------------------------------Insert_Function-------------------------------------*/


void RBTInsert(RBTNode **treeRoot,void * data,valueType key,int (*comparator)(valueType,valueType)){

	RBTNode * z = RBTnewNode(data,key);
	RBTNode * y = GUARD;
	RBTNode * x = *treeRoot;

	while(x!=GUARD){
		y  = x;
		if((*comparator)(GetKey(z),GetKey(x))<=0 )				/*For alphabetical tree :(*comparator)*/
			x = x->left;
		else if((*comparator)(GetKey(z),GetKey(x))>0 )
			x = x->right;
	}

	z->parent = y; 
	if(y==GUARD)
		*treeRoot  = z;
	else if((*comparator)(GetKey(z),GetKey(y))<=0)
		y->left  = z;
	else if((*comparator)(GetKey(z),GetKey(y))>0)
		y->right = z;

	RBTInsertFixUP(treeRoot,z);								/*Fix any possible violation of colors and balance the tree */

}

void RBTInsertFixUP(RBTNode **treeRoot,RBTNode * z){

	RBTNode * y;
	while(GetColor(GetParent(z)) == RED){					/*iterate z's parent color is red*/

		if(GetParent(z) == GetGrandParent(z)->left){

			y = GetGrandParent(z)->right;					/*Find uncle and store it in y */

			if(GetColor(y) == RED){							/*Case 1: Uncle is red ->RECOLOR*/				

				SetColor(GetParent(z),BLACK);				//Change color of parent and uncle as BLACK
				SetColor(y,BLACK);
				SetColor(GetGrandParent(z),RED);			//Change color of grandparent as RED
				z = GetGrandParent(z);						//Move z to grandparent

			}else{											/*Case 2:Uncle is black*/

				if(z == GetParent(z)->right){				//If z is on the right 
					z = GetParent(z);						//Move z to his parent
					RotateLeft(treeRoot,z);					//Left rotate z
				}
															/*Case 2:Uncle is black and z is on the left ->RECOLOR */
				SetColor(GetParent(z),BLACK);				//Make parent black
				SetColor(GetGrandParent(z),RED);			//Make gparent Red
				RotateRight(treeRoot,GetGrandParent(z));	//Right Rotate
				
			} 
				
		}else{												/*As above , but changed left with right because there are symmetrical */

			y = GetGrandParent(z)->left;

			if(GetColor(y) == RED){

				SetColor(GetParent(z),BLACK);
				SetColor(y,BLACK);
				SetColor(GetGrandParent(z),RED);
				z = GetGrandParent(z);

			}else{
				if(z == GetParent(z)->left){
					z = GetParent(z);
					RotateRight(treeRoot,z);
				}

				SetColor(GetParent(z),BLACK);
				SetColor(GetGrandParent(z),RED);
				RotateLeft(treeRoot,GetGrandParent(z));
				
			} 
				

		}
	}
	SetColor(*treeRoot,BLACK);							/* Make root always BLACK !!!! */
}

/*---------------------------------Destructors----------------------------------*/


void RBTDestroyNode(RBTNode * node){
	free(node->key);
	free(node);
}


void destroyGuard(){
	free(GUARD);
}


void RBTDestroyTree(RBTNode * treeRoot){
	
	RBTDestroyAllNodes(treeRoot);
}

void RBTDestroyAllNodes(RBTNode * treeRoot){

	if(treeRoot == GUARD)
		return;

	RBTDestroyAllNodes(treeRoot->left);
	RBTDestroyAllNodes(treeRoot->right);

	RBTDestroyNode(treeRoot);
}

void destroyDataStructures(){
	destroyGuard();
}