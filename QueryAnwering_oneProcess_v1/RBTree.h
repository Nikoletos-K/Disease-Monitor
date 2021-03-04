enum nodeColor { BLACK, RED };

typedef enum nodeColor nodeColor;
typedef struct Node RBTNode;
typedef char * valueType;

RBTNode * GUARD ;				/* Global pointer for guard */

typedef struct Node {

	RBTNode * parent,*left,*right;
	enum nodeColor color;
	valueType key;

	void * data;

} RBTNode;

int (*comparator)(valueType value,valueType key);

RBTNode * createGuard();
void destroyGuard();

RBTNode * RBTConstruct();
RBTNode * RBTnewNode(void * data,valueType key);
void RBTInitialiseKey(RBTNode * n,char * key);

RBTNode * GetParent(RBTNode* n);
RBTNode * GetGrandParent(RBTNode * n);
char * GetKey(RBTNode * n);
nodeColor GetColor(RBTNode * n);
void * get_RBTData(RBTNode * node);

void RotateLeft(RBTNode **treeRoot,RBTNode * x);
void RotateRight(RBTNode **treeRoot,RBTNode * x);

void RBTInsert(RBTNode **treeRoot,void * data,valueType key,int (*comparator)(valueType,valueType));
void RBTInsertFixUP(RBTNode **treeRoot,RBTNode * z);

void RBTReplaceNode(RBTNode **treeRoot,RBTNode * u,RBTNode * v);
void RBTRemoveNode(RBTNode **treeRoot,RBTNode * rmNode);
void RBTDeletionFixUP(RBTNode **treeRoot,RBTNode * n);

RBTNode * RBTFindNode(RBTNode * node,char * key,int (*comparator)(valueType,valueType));

void RBTDestroyNode(RBTNode * root);
void RBTDestroyTree(RBTNode * treeRoot);
void RBTDestroyAllNodes(RBTNode * treeRoot);

void RBTPrintTree(RBTNode * treeRoot,void (*printData)(void * data));

void RBTFindNodesBetweenKeys(RBTNode * node,int * counter,void * key1,void * key2,char * funValue,int (*comparator)(char *,char *),int (*function)(void*,char*) );
void RBTPrintTreeOnCondition(RBTNode * node,void (*printData)(void * data),int (*condition)(void*,char*));

void destroyDataStructures();
void initializeDataStructures();