/*
 * Will Baglivio
 * CSC 460
 * 3/24/22
 * Beastie Print Daemon
 *
 * This program simulates the producer/consumer bounded
 * buffer problem. It has a daemon which generates shared
 * memory and semaphores, and then looks for jobs to print
 * in a shared queue. When signaled, cleans up shared
 * resources.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <string.h>

#define MYIDS "beastieids"

// Indices of arrays of semaphores
#define MUTEX 0
#define FULL 1
#define EMPTY 2

struct queueVals
{
	int pid;
	char filename[25];
	int pos;
};

int fileExists(const char *filename);

int main(int argc, char *argv[])
{	
	// Check that user input queue size
	if (argc == 2)
	{
		// Get queue size
		int SIZE = atoi(argv[1]);

		// Ensure number is between 1 and 10
		if (1 <= SIZE && SIZE <= 10)
		{
			// Check if file exists
			if (!fileExists(MYIDS))
			{
				// Daemon not yet initialized, so proceed.
				// Ask OS for semaphores
				int sem_id = semget(IPC_PRIVATE, 3, 0770);
	
				// See if request was successful
				if (sem_id == -1)
				{
					printf("SemGet Failed.\n");
					return -1;
				}
		
				// Set semaphores
				semctl(sem_id, MUTEX, SETVAL, 1);
				semctl(sem_id, FULL, SETVAL, 0);
				semctl(sem_id, EMPTY, SETVAL, SIZE);

				// Get queue shared memory
				int shmidq = shmget(IPC_PRIVATE, (SIZE+2)*sizeof(struct queueVals), 0770);
	
				if (shmidq == -1)
				{
					printf("Could not get shared memory.\n");
					return -1;
				}

				// Attach to shared memory
				struct queueVals *shmemq;

				shmemq = (struct queueVals *) shmat(shmidq, NULL, SHM_RND);

				// Initialize the shared memory
				shmemq[SIZE].pos = 0;
				shmemq[SIZE+1].pos = 0;
			
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
				FILE *fopen(), *fp;

				if ((fp = fopen(MYIDS, "w")) == NULL)
				{
					printf(":( could not open beastieids to write.\n");
					return -1;
				}

				fprintf(fp, "%d\n", shmids);	// Stop id
				fprintf(fp, "%d\n", SIZE);	// Size of queue
				fprintf(fp, "%d\n", sem_id);	// Semaphores id
				fprintf(fp, "%d\n", shmidq);	// Queue id
				
				fclose(fp);

				/* Actual Printing code */
				
				// Run while we don't need to stop
				while (shmems[0] == 0)
				{
					p(FULL, sem_id);
					// Check if stopped after potential unblock
					if (shmems[0] == 1)
						break;
					p(MUTEX, sem_id);
					// Check if stopped after potential unblock
					if (shmems[0] == 1)
						break;

					// Get queue request and print
					struct queueVals request;
					request = shmemq[shmemq[SIZE].pos];
					printf("User %d prints: ", request.pid);

					char filename[25];
					strcpy(filename, request.filename);

					if ((fp = fopen(filename, "r")) == NULL)
					{
						printf(":( could not open requested file to read.\n");
						return -1;
					}

					// Read contents of file
					// Derived from https://www.geeksforgeeks.org/c-program-print-contents-file/
					char c;
					c = fgetc(fp);
					while (c != EOF)
					{
						printf("%c", c);
						c = fgetc(fp);
					}
					
					fclose(fp);

					// Move front of queue
					shmemq[SIZE].pos = (shmemq[SIZE].pos + 1) % SIZE;

					v(MUTEX, sem_id);
					v(EMPTY, sem_id);
				}

				/* Shared resources cleanup */
				// Give time for other processes to stop using shared resources
				sleep(1);

				if (shmdt(shmemq) == -1) printf("shmgm: ERROR in detaching.\n");
				if (shmdt(shmems) == -1) printf("shmgm: ERROR in detaching.\n");

				if ((shmctl(shmidq, IPC_RMID, NULL)) == -1) printf("ERROR in removing shmem.\n");
				if ((shmctl(shmids, IPC_RMID, NULL)) == -1) printf("ERROR in removing shmem.\n");
				if ((semctl(sem_id, 0, IPC_RMID, 0)) == -1) printf("ERROR in removing sem.\n");

				int ret;
				if ((ret = remove(MYIDS)) == -1) printf("ERROR: unable to delete beastieids.\n");

				printf("Daemon stopped.");				
			}
			else
			{	
				printf("Daemon already running.\n");
			}
		}
		else
		{
			printf("Queue size must be between 1 and 10.\n");
		}
	}
	else
	{
		printf("Must input queue size (between 1-10).\n");
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


