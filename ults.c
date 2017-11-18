#include "threadminator/threadminator.h"

void recursiva(int cant) {
    if(cant > 0) recursiva(cant - 1);
}

/* Now, let's run some simple threaded code. */
void test() {
    int i, tid;

    for (i = 0; i < 30; i++) {
        tid = ult1000_th_get_tid();
        printf("Soy el ult %d mostrando el numero %d \n", tid, i);
        usleep(5000 * i * tid); /* Randomizes the sleep, so it gets larger after a few iterations */

        recursiva(i);

        // Round Robin will yield the CPU
        if((i+tid)%5 == 0) ult1000_th_yield();
    }
}

/* Main program */
void main() {
    int i;

    ult1000_init();
    for(i=0; i < 5; i++) {
        ult1000_th_create(test);
    }
    ult1000_th_return(0);
}
