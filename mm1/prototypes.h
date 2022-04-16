#ifndef PROTOTYPES
#define PROTOTYPES

void enqueue(struct queue *myqueue, struct node *myprocess);
struct node* dequeue(struct queue *myqueue);
int fileExists(const char* filename);

#endif
