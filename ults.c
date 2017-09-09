#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum {
	MAX_ULTS = 4,
	STACK_SIZE = 0x400000,
};

struct CONTEXT {
	uint64_t rsp; // Stack pointer
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t rbx;
	uint64_t rbp;
};

struct TCB {
	struct CONTEXT context;
	enum {
		Unused, // Empty ULT - These TCB's are created at the begging of the execution and assigned on demand
		Running, // Running ULT
		Ready, // Ready ULT
	} state;
};

struct TCB ults[MAX_ULTS];
struct TCB *current_ult;

void ult_init(void);
void ult_return(int ret);
void ult_context_switch(struct CONTEXT *old, struct CONTEXT *new);
bool ult_yield(void);
static void ult_stop(void);
int ult_start(void (*f)(void));

// Initializes the ULT library
// The first thread will be... your main function!
void ult_init(void) {
	current_ult = &ults[0];
	current_ult->state = Running;
}

// TODO: add doc (undertand first :))
void __attribute__((noreturn))ult_return(int ret) {
	if (current_ult != &ults[0]) {
		current_ult->state = Unused;
		ult_yield();
		assert(!"reachable");
	}
	while (ult_yield());
	exit(ret);
}

// Yields the CPU execution to the next thread
// Returns true if the yielding was successful
// Returns false if there's a single thread
bool ult_yield(void) {
	struct TCB *selected_ult;
	struct CONTEXT *old, *new;

	selected_ult = current_ult;
	while (selected_ult->state != Ready) { // Search for next ready ult
		if (++selected_ult == &ults[MAX_ULTS]) {
			selected_ult = &ults[0];
		}

		if (selected_ult == current_ult) {
			return false;
		}
	}

	if (current_ult->state != Unused) { // Running ULT is flagged as Ready
		current_ult->state = Ready;
	}
	selected_ult->state = Running; // Selected ULT is flagged as Running

	old = &current_ult->context;
	new = &selected_ult->context;

	current_ult = selected_ult;
	ult_context_switch(old, new); // The infamous context switch

	return true;
}

// Finishes an ult
static void ult_stop(void) {
	ult_return(0);
}

// Starts an ult
int ult_start(void (*f)(void)) {
	char *stack;
	struct TCB *new_ult;

	for (new_ult = &ults[0];; new_ult++) {
		if (new_ult == &ults[MAX_ULTS]) {
			return -1;
		}
		else if (new_ult->state == Unused) {
			break;
		}
	}

	stack = malloc(STACK_SIZE);
	if (!stack) { // No memory :(
		return -1;
	}

	*(uint64_t *)&stack[STACK_SIZE -  8] = (uint64_t)ult_stop;
	*(uint64_t *)&stack[STACK_SIZE - 16] = (uint64_t)f;


	new_ult->context.rsp = (uint64_t)&stack[STACK_SIZE - 16]; // Sets the stack pointer
	new_ult->state = Ready; // The new ult is Ready

	return 0;
}


// Now, let's run some simple threaded code.
void test() {
	for(int i=0;i<5;i++) {
		puts("soy un ult");
		printf("%d \n", i);
		sleep(1);
		ult_yield();
	}
}

int main(void) {
	ult_init();
	ult_start(test);
	ult_start(test);
	ult_return(1);
}