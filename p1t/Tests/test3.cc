#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <stdio.h>
//Expected output:


using namespace std;

int g = 0;
int blue = 3;
int purple = 4;

void blueF(int *a);
void purpleF(int *a);

void blueF(int *a) {
    printf("Getting lock 2\n");
    thread_lock(2);
    printf("Getting lock 1\n");
    thread_lock(1);
    printf("Got lock\n");
    while(g == 0){
        thread_signal(2,blue);
        thread_wait(2,purple);
    }
    g--;
    int *b = new int();
    *b = *a - 1;
    if(*a > 0){
        thread_create((thread_startfunc_t) purpleF, (void *) b);
    }
    thread_unlock(2);
    thread_lock(1);
}

void purpleF(int *a) {
    printf("created?\n");
    thread_create((thread_startfunc_t) blueF, (void *) a);
     assert_interrupts_enabled();
    printf("Getting lock 1\n");
    thread_lock(1);
    assert_interrupts_enabled();
    printf("Getting lock 2\n");
    thread_lock(2);
     assert_interrupts_enabled();
    printf("Got lock\n");
    while(g == 0){
        //printf("Purple is signaling and waiting\n");
        thread_signal(2,purple);
         assert_interrupts_enabled();
        printf("Waiting on lock 2\n");
        thread_wait(2,blue); // has lock 1 waiting for blue on 2
         assert_interrupts_enabled();
    }
    g++;

    thread_unlock(2);
     assert_interrupts_enabled();
    thread_unlock(2);
     assert_interrupts_enabled();
    thread_lock(1);
     assert_interrupts_enabled();
}
int main() {
    printf("Printing\n");
    int *a = new int();
    *a = 10;
    printf("Printing\n");
    if (thread_libinit( (thread_startfunc_t) purpleF, (void *) a)) {
    cout << "thread_libinit failed\n";
    exit(0);
  }
}
