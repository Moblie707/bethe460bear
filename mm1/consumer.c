/*
 * Will Baglivio
 * CSC 460
 * 4/15/22
 * Memory Management Project One
 *
 * This program simulates the producer/consumer bounded
 * buffer problem with a RAM focus. It has a consumer which 
 * generates shared memory and semaphores, and then looks 
 * for processes to put in a simulated RAM. It is responsible
 * for keeping track of how long each job spends in the queue,
 * removing old jobs, adding new ones when possible, and keeping
 * up with jobs that need to get into the queue. When signaled, 
 * cleans up shared resources.
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <string.h>

#include "structs.h"
#include "prototypes.h"
#include "constants.h"

void putInRAM(/* in */ int size, /* inout */ char *RAM, /* inout */ struct process *myjob);
void removeFromRAM(/* in */ int size, /* inout */ char *RAM, /* inout */ struct process *myjob);

int main(int argc, char *argv[])
{	
	// Check that user input RAM size and buffer size
	if (argc == 4)
	{
		// Get RAM size and buffer size
		int rows = atoi(argv[1]);
		int cols = atoi(argv[2]);
		int buffSize = atoi(argv[3]);

		// Ensure parameters are within range
		if ((1 <= rows && rows <= 20) && (1 <= cols && cols <= 50) && (1 <= buffSize && buffSize <= 26))
		{
			// Check if file exists
			if (!fileExists(MYIDS))
			{
				// Consumer not yet initialized, so proceed.
				// Ask OS for semaphores
				int sem_id = semget(IPC_PRIVATE, 4, 0770);
	
				// See if request was successful
				if (sem_id == -1)
				{
					printf("SemGet Failed.\n");
					return -1;
				}
		
				// Set semaphores
				semctl(sem_id, MUTEX, SETVAL, 1);
				semctl(sem_id, FULL, SETVAL, 0);
				semctl(sem_id, EMPTY, SETVAL, buffSize);

				// Get buffer shared memory
				int shmidb = shmget(IPC_PRIVATE, (buffSize+2)*sizeof(struct process), 0770);
	
				if (shmidb == -1)
				{
					printf("Could not get shared memory.\n");
					return -1;
				}

				// Attach to shared memory
				struct process *shmemb;

				shmemb = (struct process *) shmat(shmidb, NULL, SHM_RND);

				// Initialize the shared memory
				shmemb[buffSize].pos = 0;
				shmemb[buffSize+1].pos = 0;
			
				// Get queue shared memory
				int shmidq = shmget(IPC_PRIVATE, sizeof(struct queue), 0770);

				if (shmidq == -1)
				{
					printf("Could not get shared memory.\n");
					return -1;
				}

				// Attach to shared memory
				struct queue *shmemq;
		
				shmemq = (struct queue *) shmat(shmidq, NULL, SHM_RND);

				// Get stop shared memory
				int shmids = shmget(IPC_PRIVATE, sizeof(int), 0770);

				if (shmids == -1)
				{
					printf("Could not get shared memory.\n");
					return -1;
				}

				// Attach to shared memory
				int *shmems;

				shmems = (int *) shmat(shmids, NULL, SHM_RND);

				// Initialize the shared memory
				shmems[0] = 0;

				// Write all shared resource ids to external file
				// Exception: queue only used within consumer, so no need
				FILE *fopen(), *fp;

				if ((fp = fopen(MYIDS, "w")) == NULL)
				{
					printf(":( could not open beastieids to write.\n");
					return -1;
				}

				fprintf(fp, "%d\n", shmids);	// Stop id
				fprintf(fp, "%d\n", buffSize);	// Size of buffer
				fprintf(fp, "%d\n", sem_id);	// Semaphores id
				fprintf(fp, "%d\n", shmidb);	// Buffer id
				
				fclose(fp);

				/* Actual Memory Management code */

				// Parent process will handle receiving requests from the buffer
				// and adding them to the shared consumer queue.
				if (fork())
				{
					// Current RAM ID
					char crid = 'A';

					// Run while we don't need to stop
					while (shmems[0] == 0)
					{
						// Try to access buffer
						p(FULL, sem_id);
						// Check if stopped after potential unblock
						if (shmems[0] == 1)
							break;
						p(MUTEX, sem_id);
						// Check if stopped after potential unblock
						if (shmems[0] == 1)
							break;

						// Get buffer request
						struct process request;
						request = shmemb[shmemb[buffSize].pos];

						// Move front of buffer
						shmemb[buffSize].pos = (shmemb[buffSize].pos + 1) % buffSize;

						v(MUTEX, sem_id);
						v(EMPTY, sem_id);

						
						//Place requested job in the queue.
						p(QUEUE, sem_id);
						if (shmems[0] == 1)
							break;

						// Allocate memory for process node
						struct node *myprocess = (struct node*) malloc(sizeof(struct node));

						// Assign RAM ID
						myprocess->p->rid = crid;
		
						// Increment RAM ID
						if (crid != 'Z')
						{
							crid++;
						}
						else
						{
							crid = 'A';
						}

						// Put into queue
						enqueue(shmemq, myprocess);
				
						v(QUEUE, sem_id);
					}
				}
				// Child process handles putting processes in RAM and printing jobs/RAM
				else
				{
					// Create RAM
					int sizeRAM = rows*cols;
					char RAM[sizeRAM];
					int i, j;

					for (i = 0; i < sizeRAM; i++)
					{
						RAM[i] = '.';
					} 

					// Run while we don't need to shutdown
					while (shmems[0] == 0)
					{
						// Gain access to queue
						p(QUEUE, sem_id);
						if (shmems[0] == 1)
							break;

						// Print header
						printf("ID thePID Size Sec\n");

						// Loop through queue to update each job
						int qsize = shmemq->size;

						for (i = 0; i < qsize; i++)
						{
							// Get job
							struct node *myjob = dequeue(shmemq);

							// Perform different action depending on if
							// it is in RAM
							if (myjob->p->inRAM)
							{
								// Decrement time remaining
								myjob->p->time = myjob->p->time - 1;

								// See if job is done
								if (myjob->p->time == 0)
								{
									// Remove from RAM
									removeFromRAM(sizeRAM, RAM, myjob->p);

									// Wake up producer
									v(0, myjob->p->psemid);
								}
								else
								{
									// Print job
									struct process *jp = myjob->p;
									printf("%c. %-6d %-4d %-3d", jp->rid, jp->pid, jp->size, jp->time);

									// Requeue
									enqueue(shmemq, myjob);
								}
							}
							else
							{
								// Try to put job in RAM
								putInRAM(sizeRAM, RAM, myjob->p);
								
								// Print job
								struct process *jp = myjob->p;
								printf("%c. %-6d %-4d %-3d", jp->rid, jp->pid, jp->size, jp->time);

								// Requeue
								enqueue(shmemq, myjob);
							}

						}

						v(QUEUE, sem_id);

						// Print RAM
						printf("Bob’s   Memory   Manager\n");
						printf("------------------------\n");

						// Header
						printf("*");
						for (i = 0; i < cols; i++)
							printf("*");
						printf("*\n");

						// Body
						for (i = 0; i < rows; i++)
						{
							printf("*");
							for (j = 0; j < cols; j++)
							{
								printf("%c",RAM[(i*rows) + j]);
							}
							printf("*\n");
						}

						// Footer
						printf("*");
						for (i = 0; i < cols; i++)
							printf("*");
						printf("*\n");

						// Sleep for 1 second
						sleep(1);
					}

					
					/* Shared resources cleanup */
					// Gain access to queue
					p(QUEUE, sem_id);

					int qsize = shmemq->size;

					// Iterate over queue to wake up producers
					for (i = 0; i < qsize; i++)
					{
						// Get job
						struct node *myjob = dequeue(shmemq);

						// Wake up producer
						v(0, myjob->p->psemid);
					}

					v(QUEUE, sem_id);


					// Give time for other processes to stop using shared resources
					sleep(1);

					if (shmdt(shmemb) == -1) printf("shmgm: ERROR in detaching.\n");
					if (shmdt(shmems) == -1) printf("shmgm: ERROR in detaching.\n");
					if (shmdt(shmemq) == -1) printf("shmgm: ERROR in detaching.\n");

					if ((shmctl(shmidb, IPC_RMID, NULL)) == -1) printf("ERROR in removing shmem.\n");
					if ((shmctl(shmids, IPC_RMID, NULL)) == -1) printf("ERROR in removing shmem.\n");
					if ((shmctl(shmidq, IPC_RMID, NULL)) == -1) printf("ERROR in removing shmem.\n");
					if ((semctl(sem_id, 0, IPC_RMID, 0)) == -1) printf("ERROR in removing sem.\n");

					int ret;
					if ((ret = remove(MYIDS)) == -1) printf("ERROR: unable to delete mmids.\n");

					printf("Consumer stopped.");
				}
			}
			else
			{	
				printf("Consumer already running.\n");
			}
		}
		else
		{
			printf("Check that rows is between 1 and 20, cols is between 1 and 50, and buffer size is between 1 and 26.\n");
		}
	}
	else
	{
		printf("Must input number of rows (1-20), number of columns (1-50), and buffer size (1-26).\n");
	}

	return 0;
}

void putInRAM(/* in */ int size, /* inout */ char *RAM, /* inout */ struct process *myjob)
{
	/* Pre: This function takes an array of characters representing RAM,
 	 * and a process that is to be put into RAM. 
 	 * Post: RAM is iterated over position by position trying to assign
 	 * a contiguous block of memory. If it succeeds, the job is written
 	 * into RAM and marked as such. Otherwise, nothing happens.
 	 */ 

	// Marks where the job's space in memory starts
	int jobPos = 0;

	// How much memory we have been able to acquire
	int totalSpace = 0;

	int i,j;

	for (i = 0; i < size; i++)
	{
		// If block is occupied, reset job position and space acquired.
		if (RAM[i] != '.')
		{
			jobPos = i+1;
			totalSpace = 0;
		}
		else
		{
			// One block of memory acquired.
			totalSpace++;
			
			// Check to see if we got all the memory we needed
			if (totalSpace == myjob->size)
			{
				// Go back and fill in RAM
				for (j = jobPos; j < (jobPos + myjob->size); j++)
				{
					RAM[j] = myjob->rid;
				}

				// Set info now that it is in RAM
				myjob->inRAM = 1;
				myjob->RAMPos = jobPos;
			}
		}
	}
}

void removeFromRAM(/* in */ int size, /* inout */ char *RAM, /* inout */ struct process *myjob)
{
	/* Pre: This function takes an array of characters representing RAM,
 	 * and a process that is to be removed from RAM.
 	 * Post: The job previously is RAM is removed by replacing its ID
 	 * with periods to show the space is now free.
 	 */ 

	int i;

	for (i=myjob->RAMPos; i < (myjob->RAMPos + myjob->size); i++)
	{
		RAM[i] = '.';
	}

	myjob->inRAM = 0;
}




















