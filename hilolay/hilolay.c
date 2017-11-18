#include "hilolay.h"

/* Initializes the ULT library
 * The first thread will be... your main function! */
void lib_init(void) {
    current_ult = &ults[0];
    current_ult->context = malloc(sizeof(ucontext_t));
    lib_write_TCB(current_ult, MAIN_THREAD_ID, RUNNING);
    current_ult->burst_start = lib_get_time();
    lib_round_robin_init();
}

/* __attribute__((noreturn)) tells the compiler that this function won't return
 * in order to avoid compilation warnings */
void __attribute__((noreturn)) th_return(int ret) {
    if (!lib_is_main_thread(current_ult)) {
        current_ult->state = FREE;
        th_yield();

        /* This line should never be reached. If that's the case, there's a bug */
        assert(!"reachable");
    }

    /* Yields the CPU until there are no more threads */
    while (th_yield());
    exit(ret);
}

/* Yields the CPU execution to the next thread
 * Returns true if the yielding was successful
 * Returns false if there's a single thread */
bool th_yield(void) {
    struct TCB *selected_ult;
    ucontext_t *old, *new;

    selected_ult = lib_get_next_ult();

    /* there's not other ULT to schedule */
    if (selected_ult == NULL) {
        return false;
    }

    if (current_ult->state != FREE) { /* Running ULT is flagged as Ready */
        lib_enqueue(current_ult);
    }
    lib_summarize_burst();

    selected_ult->state = RUNNING; /* Selected ULT is flagged as Running */
    selected_ult->burst_start = lib_get_time();

    old = current_ult->context;
    new = selected_ult->context;

    current_ult = selected_ult;
    swapcontext(old, new); /* The infamous context switch */

    return true;
}

/* Returns the next TCB, based on the scheduling algorithm */
struct TCB* lib_get_next_ult() {
    /* Gets the first */
    struct TCB *selected_ult = READY_QUEUE_HEAD;

    /* If the queue is not empty, the new head is tne second element */
    if(READY_QUEUE_HEAD != NULL) {
        READY_QUEUE_HEAD = READY_QUEUE_HEAD->next;
    }

    return selected_ult;
}

/* Finishes an ult */
static void th_stop(void) {
    lib_summarize_burst();
    printf("I am ult number %d. Total execution time: %d. Last burst: %d\n", th_get_tid(), current_ult->execution_time, current_ult->last_burst);
    lib_log("Finishing thread");
    th_return(0);
}

/* Wraps the code of the thread in order to summarize its info before it actually finishes */
void th_wrapper(void (*ult_function)(void)) {
    ult_function();
    th_stop();
}

/* Starts an ult */
int th_create(void (*f)(void)) {
    struct TCB *new_ult;

    for (new_ult = &ults[0];; new_ult++) {
        if (new_ult == &ults[MAX_ULTS]) {
            lib_log("Cannot create a new ULTs!");
            return ERROR_TOO_MANY_ULTS;
        } else if (new_ult->state == FREE) {
            break;
        }
    }

    th_create_context(new_ult, f);
    lib_write_TCB(new_ult, NEXT_ID++, READY);
    lib_enqueue(new_ult);

    lib_log("New thread was created");
    if(SCHEDULE_IMMEDIATELY) {
        th_yield();
    }

    return 0;
}

/* Creates the context of a new thread */
void th_create_context(struct TCB* new_ult, void (*f)(void)) {
    new_ult->context = malloc(sizeof(ucontext_t));

    // Gets the current context as a reference, to be overridden
    getcontext(new_ult->context);

    new_ult->context->uc_link = 0;
    new_ult->context->uc_stack.ss_sp = malloc(STACK_SIZE);
    new_ult->context->uc_stack.ss_size = STACK_SIZE;
    makecontext(new_ult->context, (void*)&th_wrapper, 1, f);
}

/* Enqueues the sent TCB */
void lib_enqueue(struct TCB* ult) {
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
void lib_round_robin_init() {
    if(QUANTUM == 0) return;

    struct sigaction action;

    /* Set up the structure to specify the action. */
    action.sa_handler = (void*) lib_end_of_quantum_handler;
    action.sa_flags = SA_NODEFER;
    sigemptyset(&action.sa_mask);

    sigaction(SIGALRM, &action, 0);

    /* If QUANTUM = 0 the alarm is disabled */
    alarm(QUANTUM);
}

/* Handles the end of a quantum */
void lib_end_of_quantum_handler() {
    printf("I am ult number %d switching context \n", th_get_tid());
    alarm(QUANTUM);
    th_yield();
    printf("Restarting ult number %d\n", th_get_tid());
}

/* Returns the running thread id */
int th_get_tid(void) {
    return current_ult->id;
}

/* Writes administrative info of a TCB */
void lib_write_TCB(struct TCB* tcb, int id, enum State state) {
    tcb->state = state;
    tcb->id = id;
    tcb->next = NULL;
    tcb->burst_start = 0;
    tcb->last_burst = 0;
    tcb->execution_time = 0;
}

/* Prints a message */
void lib_log(char *message) {
    if (VERBOSE) {
        printf("\n** %s **\n\n", message);
    }
}

/* Checks if a ult is the main thread */
bool lib_is_main_thread(struct TCB* ult) {
    return ult == &ults[0];
}

/* Returns current time */
int lib_get_time() {
    return (int)time(NULL);
}

/* Saves a summary of the next burst */
void lib_summarize_burst() {
    int last_burst = lib_get_time() - current_ult->burst_start;
    current_ult->execution_time += last_burst;
    current_ult->last_burst = last_burst;
}
