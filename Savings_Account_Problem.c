


struct Node {
	int data;
	struct Node *next;
};

//The position of the various semaphores that we are using in the semaphore array from semget
#define SEMAPHORE_MUTEX	0
#define SEMAPHORE_wlist 1
#define NUMBER_OF_SEMAPHORES 2

//LinkedList manipulation procedures
void AddToEndOfList(struct Node A, int val);
void DeleteFirstElement(struct Node A);
int FirstElementVal(struct Node A);

void semaphore_wait();
void semaphore_signal();
int create_semaphore(int value);

//Shared memory struct to store shared variables 
struct shared_variable_struct {
	int int wcount :=0;
	int balance :=500;
	struct Node list :=NULL;
}

int main() {
	unsigned short semaphore_init_values[NUMBER_OF_SEMAPHORES];
	semaphore_init_values
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