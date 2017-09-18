# Threadminator

Tiny user level threads library, for education purposes. The library names are based on T1000, but I expect to find a better name soon :)

## How can I run it?

- CLion: Import the project and take advantage of the "new" makefile format. Run -> run 'ults'
- Your favorite editor: Just code, and run `make clean && make`. Then `./ults`

## Project structure

This is still a little green (pun intended), but for now:
- `ults.c` is the main project
- `ult1000_th_context_switch.S` is an assembly function that switches the context between two ULTs.
- `threadminator/threadminator.c` is the actual library. It's supposed to contain all the code related to ULTs administration. The naming convention is `ult1000_func` for functions related to the library behavior, and `ult1000_th_func` for thread functions. I'm eager to find a better one, submit an issue!

A small example of code:

```
#include "threadminator/threadminator.h"

void func() {
    puts('Do you have a photograph of John?');

    // In case you want to make the library scheduling work
    ult1000_th_yield();
}


int main(void) {
    // Inits the library
    ult1000_init();

    // Creates a thread that will run func. Right now, you can create up to 49 threads (50, including main).
    ult1000_th_create(func);

    // Finishes the thread. The execution of the other threads won't stop until they finish
    ult1000_th_return(0);
}

```

## Contribute

- Fork the repo
- Check the issues. Add whatever you consider that can be useful
- Assign yourself an issue and submit a PR with the changes.
- Please consider avoiding refactors without coordinating with the rest of the team, in order to merge stuff smoothly. Don't be a lone wolf!

## Green threads

![image](https://user-images.githubusercontent.com/1786754/30241000-882ad5b6-9551-11e7-9a21-25d017386334.png)


(Originally forked from https://github.com/mpu/gthreads)
