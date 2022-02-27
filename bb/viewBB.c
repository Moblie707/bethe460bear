/*
 * Will Baglivio
 * CSC 460
 * 2/17/22
 * viewBB
 * This views the Shared Bulletin Board
 * and prints its contents to the screen.
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

	// Print out each entry in the bulletin board
	struct bbStruct temp;
	int i;
	
	for (i = 0; i < numEntries; i++)
	{
		// Get entry
		temp = shmem[i];
		
		// Print entry
		printf("%2d: %20s| %8d| %30s|\n", temp.id, temp.name, temp.favNum, temp.favFood);
	}


	// Detach from the shared memory
	if (shmdt(shmem) == -1) printf("shmgm: ERROR in detaching.\n");

	return 0;
}
