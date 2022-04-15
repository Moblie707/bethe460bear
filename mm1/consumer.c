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

#define MYIDS "mmids"

// Indices of arrays of semaphores
#define MUTEX 0
#define FULL 1
#define EMPTY 2
#define QUEUE 3

#include "structs.h"
#include "prototypes.h"

int fileExists(const char *filename);

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

						// Allocate memory for process node
						struct node *myprocess = (struct node*) malloc(sizeof(struct node));

						// Put into queue
						enqueue(shmemq[0], myprocess);
				
						v(QUEUE, sem_id);
					}

					/* Shared resources cleanup */
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
				// Child process handles putting processes in RAM and printing jobs/RAM
				else
				{


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
}

int fileExists(const char* filename)
{
	// This function checks if a file exists given
	// the name of the file.
	// 
	// Pulled from: https://www.delftstack.com/howto/c/c-check-if-file-exists/
	
	struct stat buffer;
	int exist = stat(filename, &buffer);
	
	if (exist == 0)
		return 1;
	else
		return 0;
}

p(int s,int sem_id)
{
	struct sembuf sops;

	sops.sem_num = s;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	if((semop(sem_id, &sops, 1)) == -1) printf("%s", "'P' error\n");
}

v(int s, int sem_id)
{
	struct sembuf sops;

	sops.sem_num = s;
	sops.sem_op = 1;
	sops.sem_flg = 0;
	if((semop(sem_id, &sops, 1)) == -1) printf("%s","'V' error\n");
}


