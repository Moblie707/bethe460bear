/*
 * Will Baglivio
 * CSC 460
 * 2/2/22
 * Alpha^Alpha
 */

#include <stdio.h>

int main(int argc, char * argv[])
{
	// Make sure user entered correct number of arguments
	if (argc != 2)
	{
		printf("enter a number between 1 and 26 on the command line.\n");
	}
	else
	{
		// Get number of processes to create
		int N = atoi(argv[1]);

		// Ensure the number is between 1 and 26
		if (N < 1 || N > 26)
		{
			printf("Please try again and enter a number between 1 and 26.\n");

			return 0;
		}

		// Parent Id
		int pid = getpid();

		// Create N-1 processes
		int i;
		for (i = 0; i < (N-1); i++)
		{
			// Only create process if parent
			if (pid == getpid())
			{
				fork();
			}
		}

		// Print process N times
		int j,k,m,x;
		for (i = 0; i < N; i++)
		{
			printf(":%d\n",getpid());

			for (j = 0; j < 1000; j++)
			{
				for (k = 0; k < 1000; k++)
				{
					for (m = 0; m < 100; m++)
					{
						x = j*k*m;
					}
				}
			}
		}
	}

	return 0;
}
