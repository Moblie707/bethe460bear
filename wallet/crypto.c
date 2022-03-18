/*
 * Will Baglivio
 * CSC 460
 * 3/17/22
 * Crypto Wallet
 * This program simulates the writer/reader synchronization problem.
 * It allows 32 processes to withdraw and deposit variable amounts of
 * 'crypto' from a shared integer in memory. The program accomplishes
 * the synchronization using semaphores. The actual process is 
 * distributed over different function calls indicated by command line
 * arguments as documented below.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <string.h>
#include <math.h>

#define MYIDS "cryptodata"
#define COINS shmem[0]

int fileExists(const char *filename);

int main(int argc, char *argv[])
{
	// Perform different functions based on what argument the user passes
	
	// No-Args call
	if (argc == 1)
	{
		// This program call initializes the shared memory and semaphores
		// and stores their ids to a file, unless already done so.
		
		// Check if file exists
		if (!fileExists(MYIDS))
		{
			// Wallet not yet initialized, so proceed.
			// Ask OS for semaphores
			int sem_id = semget(IPC_PRIVATE, 1, 0777);

			// See if request was successful
			if (sem_id == -1)
			{
				printf("SemGet Failed.\n");
				return -1;
			}

			// Set semaphore to 1
			semctl(sem_id, 0, SETVAL, 1);

			// Get shared memory and id
			int shmid, *shmem;

			shmid = shmget(IPC_PRIVATE, sizeof(int), 0777);

			if (shmid == -1)
			{
				printf("Could not get shared memory.\n");
				return -1;
			}

			// Attach to shared memory
			shmem = (int *) shmat(shmid, NULL, SHM_RND);

			// Initialize shared memory
			COINS = 1000;

			// Detach from shared memory
			if (shmdt(shmem) == -1)
			{
				printf("shmgm: ERROR in detaching.\n");

			}

			// Now write semaphore id and shared memory to external file
			FILE *fopen(), *fp;

			if ((fp = fopen(MYIDS, "w")) == NULL)
			{
				printf(":( could not open cryptodata to write.\n");
				return -1;
			}

			fprintf(fp, "%d\n", shmid);
			fprintf(fp, "%d", sem_id);

			fclose(fp);
		}
		else
		{
			printf("Crypto Wallet is already initialized and ready for use.\n");

		}

	}
	// Otherwise, an argument is passed
	else if (argc == 2)
	{
		char * arg = argv[1];
		
		// Call to cleanup shared resources
		if (strcmp(arg,"cleanup") == 0)
		{
			// First check if file exists
			if (fileExists(MYIDS))
			{
				// Get shared memory and semaphore id from file
				FILE *fp;
				int ret, sem_id, shmid, *shmem;

				if ((fp = fopen(MYIDS, "r")) == NULL)
				{
					printf(":( could not open cryptodata to read.\n");
					return -1;
				}

				fscanf(fp, "%d", &shmid);
				fscanf(fp, "%d", &sem_id);
				fclose(fp);
				
				// Attach to the shared memory
				shmem = (int *) shmat(shmid, NULL, SHM_RND);
				
				// Print coins
				printf("The wallet has %d coins.\n", COINS);

				// Detach and clean-up shared memory
				if (shmdt(shmem) == -1)
					printf("shmgm: ERROR in detaching.\n");
				
				if ((shmctl(shmid, IPC_RMID, NULL)) == -1)
					printf("ERROR removing shmem.\n");

				if ((semctl(sem_id, 0, IPC_RMID, 0)) == -1)
					printf("ERROR removing sem.\n");

				// Remove file
				if ((ret = remove(MYIDS)) == -1)
					printf("ERROR: unable to delete cryptodata.\n");
			}
			else
			{
				printf("Nothing to cleanup!\n");
			}
		
		}
		// Call to print coins
		else if (strcmp(arg,"coins") == 0)
		{
			// First check that file exists
			if (fileExists(MYIDS))
			{
				// Get shared memory and id from file
				FILE *fp;
				int shmid, *shmem;

				if ((fp = fopen(MYIDS, "r")) == NULL)
				{
					printf(":( could not open cryptodata to read.\n");
					return -1;
				}

				fscanf(fp, "%d", &shmid);

				// Attach to the shared memory
				shmem = (int *) shmat(shmid, NULL, SHM_RND);

				// Print coins
				printf("The wallet has %d coins.\n", COINS);

				// Detach
				if (shmdt(shmem) == -1)
					printf("shmgm: ERROR in detaching.\n");
			}
			else
			{
				printf("No wallet to view. Run program with no arguments.\n");
			}
		}
		// Call to perform withdraws and deposits
		else
		{
			// First check that file exists
			if (fileExists(MYIDS))
			{
				// Get number of times to loop
				int N = atoi(argv[1]);

				// Ensure number is between 1 and 100
				if (N < 1 || N > 100)
				{
					printf("Number of times to deposit/withdraw must be between 1 and 100.\n");
				}
				else
				{
					// Get shared memory and semaphore ids from file
					FILE *fp;
					int ret, sem_id, shmid, *shmem;

					if ((fp = fopen(MYIDS, "r")) == NULL)
					{
						printf(":( could not open cryptodata to read.\n");
						return -1;
					}

					fscanf(fp, "%d", &shmid);
					fscanf(fp, "%d", &sem_id);
					fclose(fp);
				
					// Attach to the shared memory
					shmem = (int *) shmat(shmid, NULL, SHM_RND);

					// Create 16 processes and give unique values
					int value = 0;
					int i;

					for (i = 0; i < 15; i++)
					{
						if (fork() != 0)
							break;

						value++;
					}

					// Amount to deposit/withdraw
					int amount = (int) pow((double) 2, (double) value);

					// Create parent and child processes
					// Parent
					if (fork())
					{
						// Deposit N times
						for (i = 0; i < N; i++)
						{
							// Try to enter critical section
							p(0, sem_id);
			
							// Get old coin count
							int oldcoins = COINS;
	
							// Add amount to coins
							COINS += amount;

							// Print action
							printf("%d + %d = %d\n", oldcoins, amount, COINS);
	
							// Exit critical section
							v(0, sem_id);
						}
					}
					// Child
					else
					{
						// Withdraw N times
						for (i = 0; i < N; i++)
						{
							// Try to enter critical section
							p(0, sem_id);
				
							// Get old coin count
							int oldcoins = COINS;
	
							// Subtract amount from coins
							COINS -= amount;
	
							// Print action
							printf("		%d - %d = %d\n", oldcoins, amount, COINS);
	
							// Exit critical section
							v(0, sem_id);
						}

					}

					// Detach from shared memory
					if (shmdt(shmem) == -1)
						printf("shmgm: ERROR in detaching.\n");
				}
			}
			else
			{
				printf("No wallet to access. Run program with no arguments.\n");
			}
		}

	}
	else
	{
		printf("Either zero, or one argument is accepted for this program: nothing, an integer between 1 and 100, 'coins', or 'cleanup'.\n");
	}


	return 0;
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













