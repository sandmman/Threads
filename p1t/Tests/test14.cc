#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <stdio.h>
//Expected output:


using namespace std;
void one(){
    thread_lock(0);
    printf("one wait\n");
    thread_wait(0,0);
    printf("one awake\n");
    thread_unlock(0);
    printf("one done\n");
}
void two(){
    thread_lock(0);
    printf("two yield\n");
    thread_yield();
    printf("two returns\n");
    thread_unlock(0);
    printf("two done\n");
}
void two(){
    thread_signal(0,0);
    printf("three signal\n");
    thread_lock(0);
    printf("three gets lock\n");
    thread_unlock(0);
    printf("two done\n");
}
void start(){
    thread_create((thread_startfunc_t) one, NULL);
    thread_create((thread_startfunc_t) two, NULL);
    thread_create((thread_startfunc_t) three, NULL);

}
int main() {
    thread_libinit( (thread_startfunc_t) start, NULL);
}
