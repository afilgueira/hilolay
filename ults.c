#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Error code sent when you cannot create more ULTs
static const int ERROR_TOO_MANY_ULTS = -1;

// If this option is true, the threads will start executing immediately after being created
static const bool SCHEDULE_IMMEDIATELY = true;

// The main thread ID
static const int MAIN_THREAD_ID = 0;

// ID for a new ULT
static int NEXT_ID = 1;

enum {
    MAX_ULTS = 50,
    STACK_SIZE = 0x400000,
};

/* The registers RBX, RBP, RDI, RSI, RSP, R12, R13, R14, and R15
 * are considered nonvolatile and must be saved and restored by a function that uses them.
 */
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
    int id;
    struct CONTEXT context;
    enum {
        EMPTY, // Empty ULT - These TCB's are created at the beginning of the execution and assigned on demand
        RUNNING, // Running ULT
        READY, // Ready ULT
    } state;
};

struct TCB ults[MAX_ULTS];
struct TCB *current_ult;

// Lib functions
void ult_lib_init(void);
void ult_th_return(int ret);
void ult_th_context_switch(struct CONTEXT *old, struct CONTEXT *new);
bool ult_th_yield(void);
int ult_th_start(void (*f)(void));
static void ult_th_stop(void);
int ult_th_get_tid(void);
struct TCB* ult_lib_get_next_ult();

// Helper functions
void print_error(char *);

// Initializes the ULT library
// The first thread will be... your main function!
void ult_lib_init(void) {
    current_ult = &ults[0];
    current_ult->state = RUNNING;
    current_ult->id = MAIN_THREAD_ID;
}

// TODO: add doc (understand first :))
void __attribute__((noreturn)) ult_th_return(int ret) {
    if (current_ult != &ults[0]) {
        current_ult->state = EMPTY;
        ult_th_yield();
        assert(!"reachable");
    }
    while (ult_th_yield());
    exit(ret);
}

// Yields the CPU execution to the next thread
// Returns true if the yielding was successful
// Returns false if there's a single thread
bool ult_th_yield(void) {
    struct TCB *selected_ult;
    struct CONTEXT *old, *new;

    selected_ult = ult_lib_get_next_ult();

    // there's not other ULT to schedule
    if (selected_ult == current_ult) {
        return false;
    }

    if (current_ult->state != EMPTY) { // Running ULT is flagged as Ready
        current_ult->state = READY;
    }
    selected_ult->state = RUNNING; // Selected ULT is flagged as Running

    old = &current_ult->context;
    new = &selected_ult->context;

    current_ult = selected_ult;
    ult_th_context_switch(old, new); // The infamous context switch

    return true;
}

// Returns the next TCB, based on the scheduling algorithm
struct TCB* ult_lib_get_next_ult() {
    struct TCB *selected_ult = current_ult;
    while (selected_ult->state != READY) { // Search for next ready ult
        if (++selected_ult == &ults[MAX_ULTS]) {
            selected_ult = &ults[0];
        }

        if (selected_ult == current_ult) {
            // We don't have other ULTs to schedule, so let's continue with the same
            break;
        }
    }
    return selected_ult;
}

// Finishes an ult
static void ult_th_stop(void) {
    ult_th_return(0);
}

// Starts an ult
int ult_th_start(void (*f)(void)) {
    char *stack;
    struct TCB *new_ult;

    for (new_ult = &ults[0];; new_ult++) {
        if (new_ult == &ults[MAX_ULTS]) {
            print_error("Cannot create a new ULTs!");
            return ERROR_TOO_MANY_ULTS;
        } else if (new_ult->state == EMPTY) {
            break;
        }
    }

    stack = malloc(STACK_SIZE);
    if (!stack) { // No memory :(
        print_error("Not enough memory to create a new stack :(");
        return -1;
    }

    *(uint64_t *) &stack[STACK_SIZE - 8] = (uint64_t) ult_th_stop;
    *(uint64_t *) &stack[STACK_SIZE - 16] = (uint64_t) f;


    new_ult->context.rsp = (uint64_t) &stack[STACK_SIZE - 16]; // Sets the stack pointer
    new_ult->state = READY; // The new ult is Ready
    new_ult->id = NEXT_ID++;

    if(SCHEDULE_IMMEDIATELY) {
        ult_th_yield();
    }

    return 0;
}

int ult_th_get_tid(void) {
    return current_ult->id;
}

// Prints a formatted error
void print_error(char* error) {
    printf("\n** %s **\n\n", error);
}


// Now, let's run some simple threaded code.
void test() {
    for (int i = 0; i < 10; i++) {
        printf("Soy el ult %d mostrando el numero %d \n", ult_th_get_tid(), i);
        usleep(500);
        ult_th_yield();
    }
}

int main(void) {
    ult_lib_init();
    for(int i=0; i < 100; i++) {
        ult_th_start(test);
    }
    ult_th_return(0);
}
