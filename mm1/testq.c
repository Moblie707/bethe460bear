#include "stdio.h"
#include <stdlib.h>
#include "structs.h"
#include "prototypes.h"

int main()
{
	struct queue *myqueue = (struct queue*) malloc(sizeof(struct queue));


	struct node *anode = (struct node*) malloc(sizeof(struct node));
	anode->p.pid = 0;
	enqueue(myqueue, anode);
	
	struct node *dnode = dequeue(myqueue);
	printf("%d\n",(dnode->p).pid);
	
	free(myqueue);

	return 0;
}






