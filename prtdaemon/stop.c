/*
 * Will Baglivio
 * CSC 460
 * 3/24/22
 * Beastie Print Daemon
 * 
 * This program tells the daemon to clean up
 * its shared resources and shutdown.
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

int fileExists(const char *filename);

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
			printf(":( could not open beastieids to read.\n");
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
		printf("No daemon to stop.\n");
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
