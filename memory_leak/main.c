#include "../hilolay/hilolay.h"

void leak() {
    void* mem = malloc(500000);
}

/* Main program */
void main() {
    int i;

    lib_init();
    for(i=0; i < 200; i++) {
        th_create(leak);
        usleep(5000);
    }
    th_return(0);
}
