#ifndef HILOLAY_H
#define HILOLAY_H

#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ucontext.h>
#include <unistd.h>

/* Error code sent when you cannot create more ULTs */
#define ERROR_TOO_MANY_ULTS -1

/* To make the threads start executing immediately after being created */
#define SCHEDULE_IMMEDIATELY true

/* To print in stdin the messages sent to "log" function */
#define VERBOSE true

/* The "main thread" ID */
#define MAIN_THREAD_ID 0

/* Round Robin Quantum (in seconds) - 0 => No Round Robin */
#define QUANTUM 2

/* ID for a new ULT */
static int NEXT_ID = 1;

/* Config options */
enum {
    MAX_ULTS = 5000,
    STACK_SIZE = 0x400000,
};

/* The possible states of a ULT */
enum State {
    FREE, // Free ULT - These TCB's are created at the beginning of the execution and assigned on demand
    RUNNING, // Running ULT
    READY, // Ready ULT
};

/* The TCB structure */
struct TCB {
    int id;
    ucontext_t* context;
    enum State state;
    struct TCB *next;
    int burst_start;
    int last_burst;
    int execution_time;
};

/* TCBs container */
struct TCB ults[MAX_ULTS];

/* Running thread */
struct TCB *current_ult;

/* First element of the ready queue */
static struct TCB *READY_QUEUE_HEAD = NULL;

/* Last element of the ready queue */
static struct TCB *READY_QUEUE_TAIL = NULL;

/* Lib functions */
void lib_init(void);
void th_return(int);
bool th_yield(void);
int th_create(void (*f)(void));
void th_create_context(struct TCB* new_ult, void (*f)(void));
static void th_stop(void);
void th_wrapper(void (*f)(void));
int th_get_tid(void);
void lib_write_TCB(struct TCB*, int, enum State);
struct TCB* lib_get_next_ult(void);
void lib_enqueue(struct TCB *);
void lib_round_robin_init(void);
void lib_end_of_quantum_handler(void);

/* Helper functions */
void lib_log(char *);
bool lib_is_main_thread(struct TCB *);
int lib_get_time();
void lib_summarize_burst();

#endif /* HILOLAY_H */
