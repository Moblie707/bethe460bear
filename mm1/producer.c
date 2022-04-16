/*
 * Will Baglivio
 * CSC 460
 * 4/16/22
 * Memory Management Project One
 * 
 * This program generates a job based on user specification,
 * size of process plus time needed. It submits this to a bounded
 * buffer that simulates RAM.
 */

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

int main(int argc, char *argv[])
{
	// Check if file exists
	if (fileExists(MYIDS))
	{
		// Get shared memory and semaphore id from file
		FILE *fp;
		int shmidb, shmids, *shmems, SIZE, sem_id, RAMSize;
		struct process *shmemb;

		if ((fp = fopen(MYIDS, "r")) == NULL)
		{
			printf(":( could not open mmids to read.\n");
			return -1;
		}

		fscanf(fp, "%d", &shmids);
		fscanf(fp, "%d", &SIZE);
		fscanf(fp, "%d", &sem_id);
		fscanf(fp, "%d", &shmidb);
		fscanf(fp, "%d", &RAMSize);
		fclose(fp);
		
		// Check that user input size of job and time needed
		if (argc != 3)
		{
			printf("Must input size of job (1-%d), and time of job (1-30).", RAMSize);
			return 0;
		}

		// Get job size and time
		int jsize = atoi(argv[1]);
		int jtime = atoi(argv[2]);

		// Check that parameters fall within specified range
		if ((jsize < 1 || jsize > RAMSize) || (jtime < 1 || jtime > 30))
		{
			printf("Must input size of job (1-%d), and time of job (1-30).", RAMSize);
			return 0;	
		}

		// Attach to the shared memory
		shmems = (int *) shmat(shmids, NULL, SHM_RND);
		shmemb = (struct process *) shmat(shmidb, NULL, SHM_RND);		

		// Create semaphore so consumer can signal producer when the job is done
		int psem_id = semget(IPC_PRIVATE, 1, 0770);

		// See if request was successful
		if (psem_id == -1)
		{
			printf("SemGet Failed.\n");
			return -1;
		}

		// Set semaphore
		semctl(psem_id, 0, SETVAL, 0);

		// Create process
		int myid = getpid();
		struct process myjob;
		myjob.pid = myid;
		myjob.psemid = sem_id;
		myjob.size = jsize;
		myjob.time = jtime;
		myjob.inRAM = 0;

		printf("%d is requesting %d blocks for %d seconds.\n", myid, jsize, jtime);

		// Put job in buffer
		// Only proceed if not shutdown
		p(EMPTY, sem_id);
		if (shmems[0] == 0)
		{
			p(MUTEX, sem_id);
			if (shmems[0] == 0)
			{
				shmemb[shmemb[SIZE+1].pos] = myjob;
				shmemb[SIZE+1].pos = (shmemb[SIZE+1].pos + 1) % SIZE;
			}
			v(MUTEX, sem_id);
			v(FULL, sem_id);
		}
		else
		{
			v(EMPTY, sem_id);
		}

		// Wait for job to finish (if not shutdown)
		if (shmems[0] == 0)
		{
			p(0, psem_id);
			if (shmems[0] == 0)
				printf("%d finished my request of %d blocks for %d seconds.\n", myid, jsize, jtime);
		}

		// Cleanup shared resources
		if (shmdt(shmems) == -1) printf("shmgm: ERROR in detaching.\n");
		if (shmdt(shmemb) == -1) printf("shmgm: ERROR in detaching.\n");

		if ((semctl(psem_id, 0, IPC_RMID, 0)) == -1) printf("ERROR in removing sem.\n");
	}
	else
	{
		printf("No queue to submit to.\n");
	}

	return 0;
}
