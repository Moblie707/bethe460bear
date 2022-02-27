/*
 * Will Baglivio
 * CSC 460
 * 2/17/22
 * addFavNum
 * This takes a number from the command line, and
 * puts it in the integer field for Will's entry
 * in our shared bulletin board.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define numEntries 17
#define myID 2

struct bbStruct
{
        int id;
        char name[20];
        int favNum;
        char favFood[30];
};

int main(int argc, char *argv[])
{
	// Make sure user entered correct number of arguments
	if (argc != 2)
	{       
		printf("Enter a number between -999999 and 99999999 on the command line.\n");
		return 0;
	}
	                                                                 
	// Get number to add to the bulletin board
	int myFavNum = atoi(argv[1]);
	                                                                               
        // Ensure number is between -999999 and 99999999
	if (myFavNum < -9999999 || myFavNum > 99999999)
	{       
		printf("Please try again and enter a number between -9999999 and 99999999.\n");
		return 0;
	}

	// Open file to get shared memory ID
	FILE *fopen(), *fp;
	int bbID;

	if((fp = fopen("/pub/os/bb/BBID.txt","r")) == NULL)
	{
		printf(":( could not open bulletin board to read.\n");
		return 0;
	}

	fscanf(fp,"%d",&bbID);
	fclose(fp);

	// Attach to shared memory
	struct bbStruct *shmem;
	shmem = (struct bbStruct *) shmat(bbID, NULL, SHM_RND);

	// Write favorite number to Will's entry
	shmem[myID-1].favNum = myFavNum;

	// Detach from the shared memory
	if (shmdt(shmem) == -1) printf("shmgm: ERROR in detaching.\n");

	return 0;
}
