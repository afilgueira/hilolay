#ifndef ULTS_THREADMINATOR_H
#define ULTS_THREADMINATOR_H

#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Error code sent when you cannot create more ULTs
static const int ERROR_TOO_MANY_ULTS = -1;

// To make the threads start executing immediately after being created
static const bool SCHEDULE_IMMEDIATELY = true;

// To print in stdin the messages sent to "log" function
static const bool VERBOSE = true;

// The "main thread" ID
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
        FREE, // Free ULT - These TCB's are created at the beginning of the execution and assigned on demand
        RUNNING, // Running ULT
        READY, // Ready ULT
    } state;
};

struct TCB ults[MAX_ULTS];
struct TCB *current_ult;

// Lib functions
void ult1000_init(void);
void ult1000_th_return(int ret);
void ult1000_th_context_switch(struct CONTEXT *old, struct CONTEXT *new);
bool ult1000_th_yield(void);
int ult1000_th_create(void (*f)(void));
static void ult1000_th_stop(void);
int ult1000_th_get_tid(void);
struct TCB* ult1000_get_next_ult();

// Helper functions
void ult1000_log(char *);

#endif //ULTS_THREADMINATOR_H
