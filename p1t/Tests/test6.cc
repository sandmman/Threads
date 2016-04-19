#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <stdio.h>
/*Expected output:


*/
using namespace std;

int g=0;
int blue = 3;
int purple = 4;

void blueF(int *a);
void purpleF(int *a);

void blueF(int *a) {
    printf("pree lockB %d\n",*a);
    thread_signal(2,blue);
    thread_wait(2,purple);
    g--;
    thread_lock(2);
    printf("blue %d\n",*a);
    while(g == 0){
        thread_signal(2,blue);
        thread_wait(2,purple);
    }
    g--;
    int *b = new int();
    *b = *a - 1;
    if(*a > 0){
        thread_create((thread_startfunc_t) purpleF, (void *) b);
        thread_create((thread_startfunc_t) purpleF, (void *) b);
    }
    thread_unlock(2);
}

void purpleF(int *a) {
    printf("pree lockP %d\n",*a);
    thread_lock(2);
    printf("purple %d\n",*a);
    while(g == 1){
        //printf("Purple is signaling and waiting\n");
        thread_signal(2,purple);
        thread_wait(2,blue);
    }
    g++;
    int *b = new int();
    *b = *a - 1;
    if(*a > 0){
        thread_create((thread_startfunc_t) blueF, (void *) b);
        thread_create((thread_startfunc_t) blueF, (void *) b);
    }
    thread_unlock(2);
    g--;
}

int main() {
    int *a = new int();
    *a = 100;
    if (thread_libinit( (thread_startfunc_t) purpleF, (void *) a)) {
    cout << "thread_libinit failed\n";
    thread_libinit( (thread_startfunc_t) purpleF, (void *) a);
    exit(1);
  }
}
