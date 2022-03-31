/*
 * Will Baglivio
 * CSC 460
 * 3/31/22
 * Bob's Bistro
 * Replicates the Dining Philosophers problem.
 * Uses 5 processes to iniate a deadlock.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

void think(int curr);
void hungry(int curr, int N, int sem_id);
void eat(int curr, int N, int sem_id);
void busy_code();

int main(int argc, char * argv[])
{
	printf("Can we get here even????");
	// 5 philosophers
	int N = 1;	

	// Semaphore stuff
	int HowManySems = N;
	int i, sem_id;
	printf("Journey starts here.");
	//  Ask OS for semaphores.
	sem_id = semget (IPC_PRIVATE, HowManySems, 0770);

	//  See if you got the request.
	if (sem_id == -1)
   	{
    		printf("%s","SemGet Failed.\n");
    		return (-1);
   	}
	printf("First step!");
	// Current semaphore
	int CURR = 0;

	// Create Child process, give unique ID, and initialize semaphores
	semctl(sem_id, CURR, SETVAL, 1);
	
	for (i = 1; i < N; i++)
	{
		if (fork() != 0)
			break;

		CURR++;
		semctl(sem_id, CURR, SETVAL, 1);
	}

	// Set random seed
	srand(getpid());
	printf("Almost there!");
	// Infinite loop
	while (N == N)
	{
		printf("I got here!");	
		// Three things all philosophers do
		think(CURR);
		hungry(CURR, N, sem_id);
		eat(CURR, N, sem_id);
	}

	return(0);
}

void think(int curr)
{
	// Philosopher thinks to eat some CPU cycles
	int i;

	for (i = 0; i < curr; i++)
	{
		printf("\t");
	}

	printf("%d THINKING", curr);

	busy_code();	
}

void hungry(int curr, int N, int sem_id)
{
	// Philosopher acquires 'left' chopstick via semaphore,
	// eats some CPU cycles,
	// then acquires 'right' chopstick via semaphore.
	int i;

	for (i = 0; i < curr; i++)
	{
		printf("\t");
	}

	printf("%d HUNGRY", curr);

	p((curr)%N, sem_id);
	busy_code();
	p((curr+1)%N, sem_id);
}

void eat(int curr, int N, int sem_id)
{
	// Philosopher eats some CPU cycles
	// releases 'left' chopstick via semaphore,
	// then releases 'right' chopstick via semaphore.
	int i;

	for (i = 0; i < curr; i++)
	{
		printf("\t");
	}

	printf("%d EATING", curr);

	busy_code();
	v((curr)%N, sem_id);
	v((curr+1)%N, sem_id);
}

void busy_code()
{
	// Eats up a variable number of CPU cycles
	int j,k,m,x;

	int var = (rand()%5)+1;
	printf("I'm here somehow...");
	for (j = 0; j < 1000*var; j++)
	{
		for (k = 0; k < 100*var; k++)
		{
			for (m = 0; m < 100*var; m++)
			{
				x = j*k*m;
			}
		}
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
