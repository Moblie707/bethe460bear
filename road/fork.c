/*
 * Will Baglivio
 * CSC 460
 * 1/28/22
 * Fork in the Road
 */

#include <stdio.h>

void family(int N, int gen, int pid);

int main(int argc, char * argv[])
{
	// Make sure user entered correct number of arguments
	if (argc != 2)
	{
		printf("Enter a number between 0 and 5 on the command line.\n");
	}
	else
	{
		// Get number generations to create
		int N = atoi(argv[1]);

		// Ensure the number is between 0 and 5
		if (N > 5 || N < 0)
		{
			printf("Please try again and enter a number between 0 and 5.\n");
			return 0;
		}

		// Print headers
		printf("Gen  PID   PPID\n");

		// Current generation
		int gen = 0;

		// Create gen 0 process
		printf("%d   %d  %d\n", gen, getpid(), getppid());

		// Create family tree of processes
		family(N, gen, getpid());
		
	}

	return 0;
}

void family(int N, int gen, int pid)
{
	/* Pre: This function is sent a number of times to occur,
 	 * the current generation, and the parent id
 	 *
 	 * Post: This function creates the given number of child
 	 * by forking that many times. Then each of those children
 	 * fork N-1 times.
  	 */

	// Only continue if positive number
	if (N > 0)
	{

		// Fork N number of times
		int i;
		for (i = 0; i < N; i++)
		{
			// Only fork if the parent
			if (pid == getpid())
			{
				fork();	
			}
		}

		// Print if child, then make more children
		if (pid != getpid())
		{
			// Increase generation
			gen++;

			printf("%d   %d  %d\n", gen, getpid(), getppid());
				
			family(N-1, gen, getpid());
		}
		else
		{
			sleep(1);
		}
	}
}
