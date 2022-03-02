/*
 * Will Baglivio
 * CSC 460
 * 3/2/22
 * Cheetahsync
 * Synchronizes N processes as specified by user
 * to print, in order, N times. Uses semaphores
 * to synchronize.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

int main(int argc, char * argv[])
{
	// Make sure user entered correct number of arguments
	if (argc != 2)
	{
		printf("Enter a number between 1 and 26 on the command line.\n");
		return 0;
	}

	// Get number of processes to create
	int N = atoi(argv[1]);

	// Ensure number is between 1 and 26
	if (N < 1 || N > 26)
	{
		printf("Please try again and enter a number between 1 and 26.\n");
		return 0;
	}

	int HowManySems, firstid;
	HowManySems = N;
	firstid = getpid();

	int i, sem_id;

	//  Ask OS for semaphores.
	sem_id = semget (IPC_PRIVATE, HowManySems, 0777);

	//  See if you got the request.
	if (sem_id == -1)
   	{
    		printf("%s","SemGet Failed.\n");
    		return (-1);
   	}

	// Current semaphore
	int CURR = 0;

	// Create Child process, give unique myID, and initialize semaphores
	char alphaID = 'A';
	
	semctl(sem_id, CURR, SETVAL, 1);
	
	for (i = 1; i < N; i++)
	{
		if (fork() != 0)
			break;

		CURR++;
		semctl(sem_id, CURR, SETVAL, 1);
		alphaID++;
	}
	
	// Loop N Times
	for (i = 0; i < N; i++)
	{
		p(CURR, sem_id);
		printf("%c: %d\n",alphaID,getpid());
		v((CURR+1)%N, sem_id);
	}

	// Clean up semaphores
	if (getpid() == firstid)
	{
    		sleep(1);
    		if ((semctl(sem_id, 0, IPC_RMID, 0)) == -1)
       			printf("%s", "Parent: ERROR in removing sem\n");

	}

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
