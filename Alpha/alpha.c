/*
 * Will Baglivio
 * CSC 460
 * 2/2/22
 * Alpha^Alpha
 */

#include <stdio.h>

int linsearch(int arr[], int n, int x);

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

		// Arrays of alphabetic labels
		char labels[26] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
		int ids[26];

		// Parent Id
		int pid = getpid();
		ids[0] = pid;

		// Create N-1 processes
		int i;
		for (i = 1; i < N; i++)
		{
			// Only create process if parent
			if (pid == getpid())
			{
				fork();
			}

			// Assign label to id
			if (pid != getpid())
			{
				ids[i] = getpid();
			}
		}

		// Perform linear search to find our label
		int id = getpid();
		int indx = linsearch(ids,N,id);
		char label = labels[indx];

		// Print process N times
		int j,k,m,x;
		for (i = 0; i < N; i++)
		{
			// Print info
			printf("%c:%d\n",label,id);

			// Busy code
			for (j = 0; j < 1000; j++)
			{
				for (k = 0; k < 100; k++)
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

int linsearch(int arr[], int n, int x)
{
	// Basic linear search to find given element, x,
	// in array of length n.
	//
	// Returns the index it was found at.

	int i;
	for (i = 0; i < n; i++)
	{
		if (arr[i] == x)
			break;
	}

	return i;
}













