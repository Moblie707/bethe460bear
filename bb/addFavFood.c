/*
 * Will Baglivio
 * CSC 460
 * 2/17/22
 * addFavFood
 * This takes a favorite food from the command line, and
 * puts it in the favorite food field for Will's entry
 * in our shared bulletin board.
 */

#include <stdio.h>
#include <string.h>
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


// Basic string length function
// Created using: https://stackoverflow.com/questions/25578886/how-to-count-the-number-of-characters-in-a-command-line-argument-by-using-and-cr
int my_strlen(char *input_string)
{
    /* Loop through all the characters in the string till null-terminator */
    int i;
    for(i=0; input_string[i] != '\0'; i++);
    return i;
}

int main(int argc, char *argv[])
{
	// Make sure user entered correct number of arguments
	if (argc != 2)
	{       
		printf("Enter your favorite food (no more than 30 characters).\n");
		return 0;
	}
	                                                                               
        // Ensure food is no more than 30 characters
        int length = my_strlen(argv[1]);

	if (length > 30)
	{       
		printf("Please try again and enter a food (no more than 30 characters).\n");
		return 0;
	}

	// Get food
	int i;
	char myFavFood[30];
	for (i = 0; i < length; i++)
	{
		myFavFood[i] = argv[1][i];
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

	// Write favorite food to Will's entry
	strcpy(shmem[myID-1].favFood, myFavFood);

	// Detach from the shared memory
	if (shmdt(shmem) == -1) printf("shmgm: ERROR in detaching.\n");

	return 0;
}
