#include "threadminator/threadminator.h"

/* Now, let's run some simple threaded code. */
void test() {
    for (int i = 0; i < 10; i++) {
        int tid = ult1000_th_get_tid();
        printf("Soy el ult %d mostrando el numero %d \n", tid, i);
        usleep(5000 * i * tid); /* Randomizes the sleep, so it gets larger after a few iterations */

        // Round Robin will yield the CPU
        // ult1000_th_yield();
    }
}

/* Main program */
int main(void) {
    ult1000_init();
    for(int i=0; i < 5; i++) {
        ult1000_th_create(test);
    }
    ult1000_th_return(0);
}
