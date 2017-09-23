#include "threadminator.h"

/* Initializes the ULT library
 * The first thread will be... your main function! */
void ult1000_init(void) {
    current_ult = &ults[0];
    current_ult->state = RUNNING;
    current_ult->id = MAIN_THREAD_ID;
}

/* __attribute__((noreturn)) tells the compiler that this function won't return
 * in order to avoid compilation warnings */
void __attribute__((noreturn)) ult1000_th_return(int ret) {
    if (current_ult != &ults[0]) {
        current_ult->state = FREE;
        ult1000_th_yield();
        assert(!"reachable");
    }
    while (ult1000_th_yield());
    exit(ret);
}

/* Yields the CPU execution to the next thread
 * Returns true if the yielding was successful
 * Returns false if there's a single thread */
bool ult1000_th_yield(void) {
    struct TCB *selected_ult;
    struct CONTEXT *old, *new;

    selected_ult = ult1000_get_next_ult();

    /* there's not other ULT to schedule */
    if (selected_ult == current_ult) {
        return false;
    }

    if (current_ult->state != FREE) { /* Running ULT is flagged as Ready */
        current_ult->state = READY;
    }
    selected_ult->state = RUNNING; /* Selected ULT is flagged as Running */

    old = &current_ult->context;
    new = &selected_ult->context;

    current_ult = selected_ult;
    ult1000_th_context_switch(old, new); /* The infamous context switch */

    return true;
}

/* Returns the next TCB, based on the scheduling algorithm */
struct TCB* ult1000_get_next_ult() {
    struct TCB *selected_ult = current_ult;
    while (selected_ult->state != READY) { /* Search for next ready ult */
        if (++selected_ult == &ults[MAX_ULTS]) {
            selected_ult = &ults[0];
        }

        if (selected_ult == current_ult) {
            /* We don't have other ULTs to schedule, so let's continue with the same */
            break;
        }
    }
    return selected_ult;
}

/* Finishes an ult */
static void ult1000_th_stop(void) {
    ult1000_th_return(0);
}

/* Starts an ult */
int ult1000_th_create(void (*f)(void)) {
    char *stack;
    struct TCB *new_ult;

    for (new_ult = &ults[0];; new_ult++) {
        if (new_ult == &ults[MAX_ULTS]) {
            ult1000_log("Cannot create a new ULTs!");
            return ERROR_TOO_MANY_ULTS;
        } else if (new_ult->state == FREE) {
            break;
        }
    }

    stack = malloc(STACK_SIZE);
    if (!stack) { /* No memory :( */
        ult1000_log("Not enough memory to create a new stack :(");
        return -1;
    }

    *(uint64_t *) &stack[STACK_SIZE - 8] = (uint64_t) ult1000_th_stop;
    *(uint64_t *) &stack[STACK_SIZE - 16] = (uint64_t) f;

    new_ult->context.rsp = (uint64_t) &stack[STACK_SIZE - 16]; /* Sets the stack pointer */
    new_ult->state = READY; /* The new ult is Ready */
    new_ult->id = NEXT_ID++;

    if(SCHEDULE_IMMEDIATELY) {
        ult1000_log("cree un hilo");
        ult1000_th_yield();
    }

    return 0;
}

/* Returns the running thread id */
int ult1000_th_get_tid(void) {
    return current_ult->id;
}

/* Prints a message */
void ult1000_log(char *message) {
    if (VERBOSE) {
        printf("\n** %s **\n\n", message);
    }
}
