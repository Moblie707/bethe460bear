/*
 * Will Baglivio
 * CSC 460
 * 4/1/22
 * Bob's Bistro
 * Replicates the Dining Philosophers problem.
 * Uses 5 philosophers with testing to avoid deadlock.
 * Main program monitors this situation for 100 seconds.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>

void busy_code();

#define MAXTIME 100

void think(int *shmem, int N);
void hunger(int *shmem, int sem_id, int SIZE, int N, int MUTEX);
void test(int *shmem, int sem_id, int SIZE, int N);
void eat(int *shmem, int sem_id, int SIZE, int N, int MUTEX);

enum states{thinking, hungry, eating, dead};

int main(int argc, char * argv[])
{
	// 5 philosophers
	int N = 5;	

	// Get shared memory and ID
	int shmid, *shmem;
	int time = N;

	shmid = shmget(IPC_PRIVATE, (N+1)*sizeof(int), 0770);
	if (shmid == -1)
	{
		printf("Could not get shared memory.\n");
		return(0);
	}

	// Attach to shared memory
	shmem = (int *) shmat(shmid, NULL, SHM_RND);

	// Initialize shared memory
	shmem[0] = thinking;
	shmem[1] = thinking;
	shmem[2] = thinking;
	shmem[3] = thinking;
	shmem[4] = thinking;
	shmem[time] = 1;

	// Semaphore stuff
	int HowManySems = N + 1;
	int i, sem_id;
	int MUTEX = N;
	
	//  Ask OS for semaphores.
	sem_id = semget (IPC_PRIVATE, HowManySems, 0770);

	//  See if you got the request.
	if (sem_id == -1)
   	{
    		printf("%s","SemGet Failed.\n");
    		return (-1);
   	}

	// Initialize MUTEX
	semctl(sem_id, MUTEX, SETVAL, 1);
	
	// Fork off main program
	if (fork() > 0)
	{
		// Run while at least one philosopher is alive
		while(shmem[0] != dead || shmem[1] != dead || shmem[2] != dead || shmem[3] != dead || shmem[4] != dead)
		{
			// Print out state of philosophers
			printf("%d. ", shmem[time]);

			for (i = 0; i < N; i++)
			{
				int state = shmem[i];

				switch(state)
				{
					case thinking:
						printf("thinking  ");
						break;

					case hungry:
						printf("hungry    ");
						break;

					case eating:
						printf("eating    ");
						break;

					case dead:
						printf("dead      ");
						break;

					default:
						printf("ERROR     ");
						break;
				}
			}	
	
			printf("\n");

			// Proceed 1 second
			sleep(1);
			shmem[time]++;
		}
		
		// Added for clarity sake
		printf("%d. dead      dead      dead      dead      dead\n", shmem[time]);

		// Detach and clean-up shared memory/semaphores
		if (shmdt(shmem) == -1) printf("shmgm: ERROR in detaching.\n");
		if ((shmctl(shmid, IPC_RMID, NULL)) == -1) printf("ERROR removing shmem.\n");
		if ((semctl(sem_id, 0, IPC_RMID, 0)) == -1) printf("ERROR in removing sem.\n");
	}
	// Children/Philosophers
	else
	{
		// Current semaphore
		int CURR = 0;

		// Create Child process, give unique ID, and initialize semaphores
		semctl(sem_id, CURR, SETVAL, 0);
	
		for (i = 1; i < N; i++)
		{
			if (fork() != 0)
				break;

			CURR++;
			semctl(sem_id, CURR, SETVAL, 0);
		}

		// Set random seed
		srand(getpid());
	
		// Philosophize while time is not 100
		while (shmem[time] < MAXTIME)
		{
			// Three things all philosophers do
			think(shmem, CURR);
			hunger(shmem, sem_id, N, CURR, MUTEX);
			eat(shmem, sem_id, N, CURR, MUTEX);
		}

		// Die
		shmem[CURR] = dead;
	}

	return(0);
}

void think(int *shmem, int N)
{
	// Think for 5-15 seconds
	sleep((rand()%11)+5);
}

void hunger(int *shmem, int sem_id, int SIZE, int N, int MUTEX)
{
	// Try to acquire forks
	p(MUTEX, sem_id);
	shmem[N] = hungry;
	test(shmem, sem_id, SIZE, N);
	v(MUTEX, sem_id);
	p(N, sem_id);
}

void test(int *shmem, int sem_id, int SIZE, int N)
{
	// See if forks can be acquired
	if (shmem[N] == hungry && shmem[(N+SIZE-1)%SIZE] != eating && shmem[(N+1)%SIZE] != eating)
	{
		shmem[N] = eating;
		v(N, sem_id);
	}
}

void eat(int *shmem, int sem_id, int SIZE, int N, int MUTEX)
{
	// Eat, then put forks down
	sleep((rand()%3)+1);

	p(MUTEX, sem_id);
	shmem[N] = thinking;
	int left = (N+SIZE-1)%SIZE;
	int right = (N+1)%SIZE;
	test(shmem, sem_id, SIZE, left);
	test(shmem, sem_id, SIZE, right);
	v(MUTEX, sem_id);
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
