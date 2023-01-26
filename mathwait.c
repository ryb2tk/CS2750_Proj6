// Renee Brandt
// Project 6: Create a child process using fork. Each process will be responsible for the following:
   // Child process takes numbers from command line arguments and put them into a dynamic array of a large enough size for those numbers. Write these numbers to a file including its PID. Then check to see if the sum of any of the numbers is 19.
   // Parent process will wait for child process to end, then return its PID along with the status of the child process (EXIT_SUCCESS or EXIT_FAILURE).
   // Shared memory will be large enough to hold two integers.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY 110613 //Create unique SHM_KEY, not from examples!
#define BUFF_SZ sizeof ( int )

void help() {
	printf("This program forks a child process that will take in integer arguments. It will output any pairs that sum up to 19 to a file named 'proj5.txt'.\n");
	printf("The parent process will check the status code of the child process (EXIT_SUCCESS if pairs found or EXIT_FAILURE if no pairs found), and write that to the file.\n");
}

int main (int argc, char ** argv) {
//Create help option using getopt
int option;
while ((option = getopt(argc, argv, "h")) != -1) {
   switch(option){
      case 'h':
         help();
         exit(EXIT_SUCCESS);
      case '?':
         if (isprint (optopt))
            printf(stderr, "Unknown option '-%c'.\n", optopt);
         else
            printf (stderr, "Unknown option character '\\x%x'.\n", optopt);
         return 1;
      default:
         help();
         exit(EXIT_FAILURE);
	}
}
//Before creating child process, create shared memory and set it to -2, -2).
int shmid = shmget (SHM_KEY, 2*BUFF_SZ, 0777 | IPC_CREAT); //Double buffer size to have enough space for two integers
if (shmid == -1) {
   printf(stderr, "Error in SHMGET");
   exit(1);
}
int* a = (int *) shmat(shmid,0,0); //first integer position in shared memory
int* b = a+1; //second integer position in shared memory
*a = -2; //set shared memory integers both to -2
*b = -2;

//Fork child process
pid_t childPid = fork();

if (childPid == 0) { //child process
   int shmid = shmget (SHM_KEY, 2*BUFF_SZ, 0777); //get shared memory segment identifier
   if (shmid == -1){
      printf(stderr, "Child: Error in SHMGET");
      exit(1);
   }
   int *cint1 = (int*)(shmat(shmid,0,0)); //Create pointers within child to shared memory
   int *cint2 = cint1+1;

   int * arglist = malloc(argc * sizeof(int));
   int i,j;
   if (arglist == NULL) { //malloc has failed
      printf("Malloc has failed.");
      return (EXIT_FAILURE);
   } else {
      for (i=0; i < argc-1; i++) { //create dynamic array of given integer arguments
         arglist[i]=atoi(argv[i+1]); //skip process name argv[0]
      }
   }
   printf("Child %d: ", getpid());
   for (i = 0; i < argc-1; i++) {
      printf("%d ", arglist[i]); //print original array in file
   }
   int noPairs = 1; //check if any pairs are found
   for (i = 0; i < argc-2; i++) { //nested loops
      for (j = 1; j < argc - 1; j++) {
         if (arglist[i] + arglist[j] == 19) {
            *cint1 = arglist[i]; //if found, save pair in shared memory
            *cint2 = arglist[j];
            noPairs = 0;
            exit(0); //exit after finding one pair
         }
      }
   }
free(arglist); //free dynamically allocated memory
   if (noPairs == 1) { //if no pairs found
      //change shared memory values to -1, -1
      *cint1 = -1;
      *cint2 = -1;
      exit(1); //indicate exit failure because no pair found
   }
} else { //parent process
   //check shared memory instead of child status
   int status = 0;
   wait(&status);
   int shmid = shmget(SHM_KEY, 2*BUFF_SZ, 0777); //get shared memory segment identifier in parent
   if (shmid == -1) {
      printf(stderr, "\nParent: Error in SHMGET");
      exit(1);
   }
   int* pint1=(int*)(shmat(shmid,0,0)); //create pointers to shared memory in parent
   int* pint2=pint1+1;
   if (*pint1==-2 || *pint2==-2) {
      printf("\nERROR: Something went wrong and the child did nothing.\n");
      exit(1);
   } else if (*pint1==-1 || *pint2==-1) {
      printf("\nNo pairs found in child.\n");
      exit(0);
   } else {
      printf("\nPairs found by child: %d %d\n", *pint1, *pint2);
   }
}

shmdt(a); //detach pointers to shared memory
shmdt(b);
shmctl(shmid,IPC_RMID,NULL); //free shared memory
return (EXIT_SUCCESS);
}
