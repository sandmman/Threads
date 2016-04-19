#include "thread.h"
#include <stdio.h>

/*Expected output:

    Printing Not Basic: 14
    Printing Basic: 7
    Thread library exiting.

*/

int x = 1;
int xCV = 2;

void not_basic(int *i){
    thread_lock(1);
    printf("Printing Not Basic: %d\n",((*i)*2));
    x = 0;
    thread_signal(1,xCV);
    //thread_yield();
    thread_unlock(1);
}

void basic(int *i){
    thread_lock(1);
    thread_create((thread_startfunc_t) not_basic, (void *) i);
    thread_yield();
    while(x != 0){
        thread_wait(1,xCV);
    }
    printf("Printing Basic: %d\n", *i);
    thread_unlock(1);
}

int main(int argc,char* argv[]){
    int i = 7;
    thread_libinit((thread_startfunc_t) basic, (void *) &i);

}
