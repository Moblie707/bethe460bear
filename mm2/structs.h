#ifndef STRUCTS
#define STRUCTS

// Information for a process
#include <stdio.h>
typedef enum {none, newp, ready, run, suspended, terminate} State;

struct process
{
	int pid;
	int psemid;
	char rid;
	int size;
	int time;
	State mystate;
	int RAMPos;
	int ctime;
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
