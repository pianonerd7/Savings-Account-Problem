#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


//key for the semaphore
#define SEMAPHORE_KEY 0XFA2B

//The position of the various semaphores that we are using in the semaphore array from semget
#define SEMAPHORE_MUTEX	0
#define SEMAPHORE_wlist 1
#define NUMBER_OF_SEMAPHORES 2

//To distinguish between withdraw and deposit when forking
#define DEPOSIT 1
#define WITHDRAW 2

//LinkedList manipulation procedures
void AddToEndOfList(struct Node A, int val);
void DeleteFirstElement(struct Node A);
int FirstElementVal(struct Node A);

void semaphore_wait();
void semaphore_signal();
int create_semaphore(int value);

int get_semid(key_t semkey);
int get_shmid(key_t shmkey);

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};

//Shared memory struct to store shared variables 
struct shared_variable_struct {
	int wcount =0;
	int balance =500;
	struct Node list =NULL;
}

//Node for linkedlist
struct Node {
	int data;
	struct Node *next;
};

int main() {
	unsigned short semaphore_init_values[NUMBER_OF_SEMAPHORES];
	//semaphore_init_values
}

void AddToEndOfList(struct Node A, int val) {

	struct Node *temp;
	
	temp = (struct Node *) malloc (sizeof(struct Node));
	temp -> Data = val;

	if (shared_variable_struct->list == NULL) {
		shared_variable_struct->list = temp;
		shared_variable_struct->list = NULL;
	}
	else {
		struct Node trav = shared_variable_struct->list;

		while (trav->next != NULL) {
			trav = trav->next;
		}

		trav->next = temp;
	}
}

void DeleteFirstElement(struct Node A) {
	if (A != NULL) {
		struct Node *newHead = A->next;
	}
}

int FirstElementVal(struct Node A) {
	return A->data;
}

void semaphore_wait(int semid, int semnumber) {
	// declare a sembuf
	struct sembuf wait_buffer;
	// We will perfom an operation on the semnumber semaphore of semid
	wait_buffer.sem_num = semnumber;
	//We will subtract 1 from the semaphore
	wait_buffer.sem_op = -1;
	wait_buffer.sem_flg = 0;
	// Perform the semaphore operation and check for errors
	if (semop(semid, &wait_buffer, 1) == -1)  {
		perror("semaphore_wait failed");
		exit(EXIT_FAILURE);
	}
}

void semaphore_signal(int semid, int semnumber) {
	// declare a sembuf
	struct sembuf signal_buffer;
	// We will perform an operation on the semnumber semaphore of semid
	signal_buffer.sem_num = semnumber;
	//We will add 1 to the semaphore
	signal_buffer.sem_op = 1;
	signal_buffer.sem_flg = 0;
	// Perform the semaphore operation and check for errors
	if (semop(semid, &signal_buffer, 1) == -1)  {
		perror("semaphore_signal failed");
		exit(EXIT_FAILURE);
	}
}