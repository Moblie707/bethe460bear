/*
 * Will Baglivio
 * CSC 460
 * 2/17/22
 * Tortoisesync
 * Synchronizes N processes as specified by user
 * to print, in order, N times. Uses shared memory
 * to synchronize.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define TURN shmem[0]

int main(int argc, char *argv[])
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

	// Get shared memory and ID
	int shmid, *shmem;

	shmid = shmget(IPC_PRIVATE, sizeof(int), 0770);
	if (shmid == -1)
	{
		printf("Could not get shared memory.\n");
		return 0;
	}

	// Attach to shared memory
	shmem = (int *) shmat(shmid, NULL, SHM_RND);

	// Initialize shared memory
	int myID = 0;	

	TURN = myID;

	//  Create Child process and give unique myID
	char alphaID = 'A';
	int i;

	for (i = 1; i < N; i++)
	{
		if (fork() != 0)
			break;

	  	myID++;
 		alphaID++;
	}

	//  Loop N Times
	int found;

	for (i = 0; i < N; i++)
	{
	     	// Repeatedly check memory until myID is found
	     
		while (TURN != myID);
     
		// It must be my turn to do something.....
		printf("%c: %d\n",alphaID,getpid());

		// Update hsared memory to allow other processes to go
		TURN = (myID+1)%N;
	}

	// Detach and clean-up the shared memory
	if (shmdt(shmem) == -1) printf("shmgm: ERROR in detaching.\n");

	if (myID == 0)
	if ((shmctl(shmid, IPC_RMID, NULL)) == -1)
		printf("ERROR removing shmem.\n");

    	return(0);
}









