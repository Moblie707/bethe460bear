/*
 * Will Baglivio
 * CSC 460
 * 2/9/22
 * Slugsync
 * Synchronizes N file processes as specified by user
 * to print, in order, N times.
 */

#include <stdio.h>

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

	FILE *fopen(), *fp;

	// Open File to write a value
	if((fp = fopen("syncfile","w")) == NULL)
    	{
    		printf(":( could not open syncfile to write.\n");
	    	return(0);
    	}

	// Write first ID into syncfile
	int myID = 0;	

	fprintf(fp,"%d",myID);
	fclose(fp);

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
	     	// Repeatedly check file until myID is found
	     	found = -1;

		while (found != myID)
		{
        		if((fp = fopen("syncfile","r")) == NULL)
	             	{
		             printf(":( could not open syncfile to read.\n");
        		     return(0);
	             	}

		        fscanf(fp,"%d",&found);
        		fclose(fp);
		}
     
		// It must be my turn to do something.....
		printf("%c: %d\n",alphaID,getpid());

		// Update file to allow otherID to go 
		if((fp = fopen("syncfile","w")) == NULL)
        	{
	      		printf(":( could not open myLittleFile to write.\n");
	        	return(0);
		}
		
		fprintf(fp,"%d",(myID+1)%N);
		fclose(fp);
	}

    	return(0);
}









