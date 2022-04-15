#ifndef STRUCTS
#define STRUCTS

// Information for a process
struct process
{
	int pid;
	int psemid;
	char rid;
	int size;
	int time;
	int inRAM;
	int RAMPos;
	int pos;
};

// Information for a process node
struct node
{
	struct process p;
	struct node *next;
};

// Queue information
struct queue
{
	int size;
	struct node *front;
	struct node *rear;
};

#endif
