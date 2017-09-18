#include "threadminator/threadminator.h"


// Now, let's run some simple threaded code.
void test() {
    for (int i = 0; i < 10; i++) {
        printf("Soy el ult %d mostrando el numero %d \n", ult1000_th_get_tid(), i);
        usleep(500);
        ult1000_th_yield();
    }
}


int main(void) {
    ult1000_init();
    for(int i=0; i < 5; i++) {
        ult1000_th_create(test);
    }
    ult1000_th_return(0);
}
