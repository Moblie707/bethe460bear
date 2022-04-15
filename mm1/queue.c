#ifndef QUEUE
#define QUEUE

/* Will Baglivio
 * 4/15/22
 * CSC 460
 * Queue
 *
 * This includes the information for a queue 'class'
 * of process nodes. A queue maintains its current size,
 * as well as a front and rear process node. You can only
 * enqueue or dequeue a process from a queue.
 */

#include "structs.h"

void enqueue(struct queue *myqueue, struct node *myprocess)
{
	// Pre: This method takes a queue with zero or more process nodes.
	// Post: A process node is inserted at the rear of the queue.

	// If queue is empty, front and rear should both
	// point to new process.
	if (myqueue->size == 0)
	{
		myqueue->front = myprocess;
		myqueue->rear = myprocess;
		myqueue->size = myqueue->size + 1;
	}
	// If queue is non-empty, add it to the rear.
	else
	{
		myqueue->rear->next = myprocess;
		myqueue->rear = myprocess;
		myqueue->size = myqueue->size + 1;
	}
}

struct node* dequeue(struct queue *myqueue)
{
	// Pre: This method takes a queue with zero or more process nodes.
	// Post: If the queue is non empty, return front node, then move
	// front node back one.

	// Get front process
	struct node *frontprocess = myqueue->front;
	
	// Only move back front of queue nonempty
	if (myqueue->size > 0)
	{
		myqueue->front = frontprocess->next;
		myqueue->size = myqueue->size - 1;
	}
	
	return frontprocess;
}

#endif
