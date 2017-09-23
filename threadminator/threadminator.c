#include "threadminator.h"

/* Initializes the ULT library
 * The first thread will be... your main function! */
void ult1000_init(void) {
    current_ult = &ults[0];
    current_ult->state = RUNNING;
    current_ult->id = MAIN_THREAD_ID;
    current_ult->next = NULL;
    ult1000_round_robin_init();
}

/* __attribute__((noreturn)) tells the compiler that this function won't return
 * in order to avoid compilation warnings */
void __attribute__((noreturn)) ult1000_th_return(int ret) {
    if (!ult1000_is_main_thread(current_ult)) {
        current_ult->state = FREE;
        ult1000_th_yield();

        /* This line should never be reached. If that's the case, there's a bug */
        assert(!"reachable");
    }

    /* Yields the CPU until there are no more threads */
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
    if (selected_ult == NULL) {
        return false;
    }

    if (current_ult->state != FREE) { /* Running ULT is flagged as Ready */
        ult1000_enqueue(current_ult);
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
    /* Gets the first */
    struct TCB *selected_ult = READY_QUEUE_HEAD;

    /* If the queue is not empty, the new head is tne second element */
    if(READY_QUEUE_HEAD != NULL) {
        READY_QUEUE_HEAD = READY_QUEUE_HEAD->next;
    }

    return selected_ult;
}

/* Finishes an ult */
static void ult1000_th_stop(void) {
    ult1000_log("Finalizando hilo");
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
    new_ult->next = NULL;

    ult1000_enqueue(new_ult);

    ult1000_log("cree un hilo");
    if(SCHEDULE_IMMEDIATELY) {
        ult1000_th_yield();
    }

    return 0;
}

/* Enqueues the sent TCB */
void ult1000_enqueue(struct TCB* ult) {
    ult->state = READY;

    /* If there are no ready ULTs */
    if(READY_QUEUE_HEAD == NULL) {
        READY_QUEUE_HEAD = ult;
    }

    /* If there is already a last ULT */
    if(READY_QUEUE_TAIL != NULL) {
        READY_QUEUE_TAIL->next = ult;
    }

    /* ULT is now the last, and the next is NULL */
    READY_QUEUE_TAIL = ult;
    READY_QUEUE_TAIL->next = NULL;
}

/* Initializes the library to listen SIGALRM */
void ult1000_round_robin_init() {
    struct sigaction action;

    /* Set up the structure to specify the action. */
    action.sa_handler = ult1000_end_of_quantum_handler;
    action.sa_flags = SA_NODEFER;
    sigemptyset(&action.sa_mask);

    sigaction(SIGALRM, &action, 0);

    /* If QUANTUM = 0 the alarm is disabled */
    alarm(QUANTUM);
}

/* Handles the end of a quantum */
void ult1000_end_of_quantum_handler() {
    printf("Soy el ult %d haciendo switch \n", ult1000_th_get_tid());
    alarm(QUANTUM);
    ult1000_th_yield();
    printf("Reiniciando el ult %d\n", ult1000_th_get_tid());
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

/* Checks if a ult is the main thread */
bool ult1000_is_main_thread(struct TCB* ult) {
    return ult == &ults[0];
}
