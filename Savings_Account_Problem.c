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

//Node for linkedlist
struct Node {
	int data;
	struct Node *next;
};

//Shared memory struct to store shared variables 
struct shared_variable_struct {
	int wcount;
	int balance;
	struct Node list;
};

void fork_process(int deposit_or_withdraw, int amount);

//LinkedList manipulation procedures
void AddToEndOfList(struct Node *A, int val);
void DeleteFirstElement(struct Node *A);
int FirstElementVal(struct Node *A);

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

void print_list(struct Node *head) {
	struct Node *current = head;

	printf("\t Beginning of list\n");
	while (current != NULL) {
		printf("\t %d\n", current->data);
		current = current ->next;
	}
	printf("\t End of list \n");
}

void debug_print_shared(struct shared_variable_struct *shared) {

	int wcount;
	int balance;
	struct Node *list;

	wcount = shared->wcount;
	balance = shared->balance;
	list = &(shared->list);

	printf("\t Share Variable status at PID %d: wcount = %d, balance = %d \n", getpid(), wcount, balance);
	print_list(list);
}

void check_linkedlist() {
	struct Node *test = malloc(sizeof(struct Node));
	test->data = 1;
	test->next = malloc(sizeof(struct Node));
	test->next->data = 2;
	test->next->next = malloc(sizeof(struct Node));
	test->next->next->data = 3;
	
	printf("start \n");
	printf("first element value is %d\n", FirstElementVal(test));

	print_list(test);

	printf("\n\n\nnew \n");
	AddToEndOfList(test, 4);

	print_list(test);

	free(test);

	printf("end\n");
}
//"-, 2, 0", "4, 0"
void main(int argc, char *argv[]) {

	printf("*** Hello world! I am %d. \n", getpid());

	union semun semaphore_values;

	//setup semaphores
	unsigned short semaphore_init_values[NUMBER_OF_SEMAPHORES];
	semaphore_init_values[SEMAPHORE_MUTEX] = 1;
	semaphore_init_values[SEMAPHORE_wlist] = 0;
	semaphore_values.array = semaphore_init_values;

	//make semaphores
	int semid = get_semid((key_t)SEMAPHORE_KEY);
	if (semctl(semid, SEMAPHORE_MUTEX, SETALL, semaphore_values) == -1) {
		perror("semctl failed");
		exit(EXIT_FAILURE);
	}

	//shared memory
	int shmid = get_shmid((key_t)SEMAPHORE_KEY);
	struct shared_variable_struct * shared_variable = shmat(shmid, 0, 0);

	//set initial values of shared memory
	shared_variable->wcount	= 0;
	shared_variable->balance = 500;
	//shared_variable->list = NULL;

	int i = 5; 
	char *number; 

	fork_process(DEPOSIT, 100);


/*
	while (argv[1][i] != '\0') {
		while (argv[1][i] != ',') {
			number 
		}
	} */

	//Wait until all the processes exit
	int j;
	for (j = 0; j < i; j++) {
		wait(NULL);
	}

	//Clean up
	if (shmdt(shared_variable) == -1) {
		perror("shmt failed");
		exit(EXIT_FAILURE);
	}
	if (shmctl(shmid, IPC_RMID, NULL) < 0) {
		perror("shmctrl failed");
		exit(EXIT_FAILURE);
	}
	if (semctl(semid, SEMAPHORE_MUTEX, IPC_RMID, semaphore_values) == -1) {
		perror("semctl failed");
		exit(EXIT_FAILURE);
	}
}

void deposit(int deposit_amount) {
	printf("***PID: %d: I am a DEPOSIT of amount %d! \n", getpid(), deposit_amount);

	//Get the semaphores and shared memory
	int semid = get_semid((key_t)SEMAPHORE_KEY);
	int shmid = get_shmid((key_t)SEMAPHORE_KEY);
	struct shared_variable_struct * shared_variable = shmat(shmid, 0, 0);

	printf("---PID: %d: D: Waiting on Mutex.\n", getpid());
	semaphore_wait(semid, SEMAPHORE_MUTEX);
	printf("---PID: %d: D: Passed Mutex.\n", getpid());
	
	shared_variable->balance = shared_variable->balance + deposit_amount;
	printf("---PID: %d: D: An amount of %d has been added to the balance.\n", getpid(), deposit_amount);
	debug_print_shared(shared_variable);

	//no withdraws at this time
	if (shared_variable->wcount	= 0) {
		printf("---PID: %d: D: Signaling MUTEX. \n", getpid());
		semaphore_signal(semid, SEMAPHORE_MUTEX);
	}
	//Still not enough balance for 1st waiting withdraw request
	else if (FirstElementVal(&(shared_variable->list)) > shared_variable->balance) {
		printf("---PID: %d: D: Signaling MUTEX. \n", getpid());
		semaphore_signal(semid, SEMAPHORE_MUTEX);
	}
	//release the earliest waiting customer
	else {
		printf("---PID: %d: D: Signaling wlist. \n", getpid());
		semaphore_signal(semid, SEMAPHORE_wlist);
	}

	printf("---PID: %d: D: End of deposit! \n", getpid());
	debug_print_shared(shared_variable);

	exit(EXIT_SUCCESS);
}

void withdraw(int withdraw_amount) {
	printf("***PID: %d: I am a WITHDRAW of amount %d! \n", getpid(), withdraw_amount);

	//Get the semaphores and shared memory
	int semid = get_semid((key_t)SEMAPHORE_KEY);
	int shmid = get_shmid((key_t)SEMAPHORE_KEY);
	struct shared_variable_struct * shared_variable = shmat(shmid, 0, 0);
	
	printf("--- PID: %d: W: Waiting on Mutex.\n", getpid());
	semaphore_wait(semid, SEMAPHORE_MUTEX);
	printf("--- PID: %d: W: Passed Mutex.\n", getpid());

	//Enough balance to withdraw
	if (shared_variable->wcount == 0 && shared_variable->balance > withdraw_amount) {
		shared_variable->balance = shared_variable->balance - withdraw_amount;

		printf("---PID: %d: W: An amount of %d has been deducted from the balance.\n", getpid(), withdraw_amount);
		debug_print_shared(shared_variable);

		printf("---PID: %d: W: Signaling MUTEX. \n", getpid());
		semaphore_signal(semid, SEMAPHORE_MUTEX);
	}
	//Either other withdrawal requests are waiting or not enough balance
	else {
		shared_variable->wcount	= shared_variable->wcount + 1;

		AddToEndOfList(&(shared_variable->list), withdraw_amount);

		debug_print_shared(shared_variable);

		printf("---PID: %d: W: Signaling MUTEX. \n", getpid());
		semaphore_signal(semid, SEMAPHORE_MUTEX);

		//Wait for a deposit
		semaphore_wait(semid, SEMAPHORE_wlist);
		printf("---PID: %d: W: Was waiting, now I'm signaled. \n", getpid());

		//Withdraw
		shared_variable->balance = shared_variable->balance	- FirstElementVal(&(shared_variable->list));
		printf("---PID: %d: W: First element value is deducted from balance. \n", getpid());
		debug_print_shared(shared_variable);

		//Remove own request from the waiting list
		DeleteFirstElement(&(shared_variable->list));
		shared_variable->wcount = shared_variable->wcount - 1;

		if (shared_variable->wcount > 1 && (FirstElementVal(&(shared_variable->list)) < shared_variable->balance)) {
			printf("---PID: %d: W: Signaling wlist. \n", getpid());
			semaphore_signal(semid, SEMAPHORE_wlist);
		}
		//This signal() is paired with the depositing customer's wait(mutex)
		else {
			printf("---PID: %d: W: Signaling MUTEX. \n", getpid());
			semaphore_signal(SEMAPHORE_MUTEX);
		}
	}

	printf("---PID: %d: W: End of withdraw! \n", getpid());
	debug_print_shared(shared_variable);

	exit(EXIT_SUCCESS);
}

void fork_process(int deposit_or_withdraw, int amount) {
	pid_t child_pid;
	child_pid = fork();

	if (child_pid == -1) {
		perror("Fork Failed");
		exit(EXIT_FAILURE);
	}
	else if (child_pid == 0) {
		//child
		if (deposit_or_withdraw == DEPOSIT) {
			deposit(amount);
		}
		else if (deposit_or_withdraw == WITHDRAW) {
			withdraw(amount);
		}
		else {
			printf("!!! Invalid transaction type! \n");
			exit(EXIT_FAILURE);
		}
	}
	else {
		//parent
		return;
	}
}

void AddToEndOfList(struct Node *A, int val) {

	struct Node *current = A;
	while (current->next != NULL) {
		current = current ->next;
	}

	current->next = malloc(sizeof(struct Node));
	current->next->data = val;
	current->next->next = NULL;
}

void DeleteFirstElement(struct Node *A) {
	if (A != NULL) {
		struct Node *newHead = A->next;
	}
}

int FirstElementVal(struct Node *A) {
	struct Node *current = A;
	
	int data = current->data;
	return data;
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

int get_semid(key_t semkey) {
	int value = semget(semkey, NUMBER_OF_SEMAPHORES, 0777 | IPC_CREAT);
	if (value == -1) {
		perror("semget failed");
		exit(EXIT_FAILURE);
	}
	return value;
}

int get_shmid(key_t shmkey) {
	int value = shmget(shmkey, sizeof(struct shared_variable_struct), 0777 | IPC_CREAT);
	if (value == -1) {
		perror("shmkey failed");
		exit(EXIT_FAILURE);
	}
	return value;
}