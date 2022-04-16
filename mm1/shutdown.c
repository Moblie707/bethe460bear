/*
 * Will Baglivio
 * CSC 460
 * 4/16/22
 * Memory Management Project One
 * 
 * This program tells the consumer and producers 
 * to clean up their shared resources and shutdown.
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
		int shmids, *shmems, SIZE, sem_id;

		if ((fp = fopen(MYIDS, "r")) == NULL)
		{
			printf(":( could not open mmids to read.\n");
			return -1;
		}

		fscanf(fp, "%d", &shmids);
		fscanf(fp, "%d", &SIZE);
		fscanf(fp, "%d", &sem_id);
		fclose(fp);

		// Attach to the shared memory
		shmems = (int *) shmat(shmids, NULL, SHM_RND);

		// Set stop to true
		shmems[0] = 1;

		// V on FULL and MUTEX
		v(FULL, sem_id);
		v(MUTEX, sem_id);

		// Detach from shared memory
		if (shmdt(shmems) == -1) printf("shmgm: ERROR in detaching.\n");
	}
	else
	{
		printf("No consumer to stop.\n");
	}	
}
