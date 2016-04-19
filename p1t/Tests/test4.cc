#include "thread.h"
#include <stdio.h>


/*Expected output:
    7
    14
    Thread library exiting.
*/
int readyVar = 0;
int ready_lock = 15;

void not_basic(int *i){
    thread_lock(ready_lock);
    while(readyVar != 1){
        thread_wait(ready_lock, readyVar);
    }
    printf("%d\n", (*i)*2);
    thread_unlock(ready_lock);
}

void basic(int *i){
    thread_lock(ready_lock);
    thread_create((thread_startfunc_t) not_basic, (void *) i);
    thread_create((thread_startfunc_t) not_basic, (void *) i);
    printf("Made Other Threads\n");
    thread_yield();
    printf("Yielded to Other Threads\n");
    readyVar = 1;
    printf("%d\n", *i);
    thread_broadcast(ready_lock, readyVar);
    thread_unlock(ready_lock);
}

int main(int argc,char* argv[]){
    int i = 7;
    thread_libinit((thread_startfunc_t) basic, (void *) &i);
    printf("Done initializing\n");
}
