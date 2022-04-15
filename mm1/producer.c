/*
 * Will Baglivio
 * CSC 460
 * 3/24/22
 * Beastie Print Daemon
 * 
 * This program generates a file with a quote of the day,
 * then submits that file to a queue 5 times, with some
 * work in between.
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
	// Check if file exists
	if (fileExists(MYIDS))
	{
		// Set random seed
		int myid = getpid();
		srand(getpid());

		// Get shared memory and semaphore id from file
		FILE *fp;
		int shmidq, shmids, *shmems, SIZE, sem_id;
		struct queueVals *shmemq;

		if ((fp = fopen(MYIDS, "r")) == NULL)
		{
			printf(":( could not open beastieids to read.\n");
			return -1;
		}

		fscanf(fp, "%d", &shmids);
		fscanf(fp, "%d", &SIZE);
		fscanf(fp, "%d", &sem_id);
		fscanf(fp, "%d", &shmidq);
		fclose(fp);

		// Attach to the shared memory
		shmems = (int *) shmat(shmids, NULL, SHM_RND);
		shmemq = (struct queueVals *) shmat(shmidq, NULL, SHM_RND);		

		// Generate file to print
		char filename[9];
		sprintf(filename, "%d.txt", myid);
		char fcmd[100];
		sprintf(fcmd, "curl -s http://api.quotable.io/random | cut -d\":\" -f4 | cut -d\"\\\"\" -f2 > %s", filename);
		system(fcmd);

		// Print and work 5 times
		int i, time;
		for (i = 0; i < 5; i++)
		{
			// Simulate working
			time = (rand()%3)+1;
			printf("User %d is working for %d seconds.\n", myid, time);
			sleep(time);

			// Request to print
			p(EMPTY, sem_id);
			p(MUTEX, sem_id);

			printf("User %d is printing %s.\n", myid, filename);
			shmemq[shmemq[SIZE+1].pos].pid = myid;
			strcpy(shmemq[shmemq[SIZE+1].pos].filename, filename);
			shmemq[SIZE+1].pos = (shmemq[SIZE+1].pos + 1) % SIZE;

			v(MUTEX, sem_id);
			v(FULL, sem_id);
		}

		printf("User %d is logging off.\n", myid);

		// Detach from shared memory
		if (shmdt(shmems) == -1) printf("shmgm: ERROR in detaching.\n");
		if (shmdt(shmemq) == -1) printf("shmgm: ERROR in detaching.\n");
	}
	else
	{
		printf("No queue to submit to.\n");
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
