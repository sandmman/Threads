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
    thread_lock(2);
    printf("blue %d\n",*a);
    while(g == 0){
        //printf("Blue is signaling and waiting\n");
        thread_signal(2,blue);
        thread_wait(2,purple);
    }
    g--;
    //printf("bl %d\n",*a);
    int *b = new int();
    *b = *a - 1;
    //delete a;
    //printf("bl %d\n",b);
    if(*a > 0){
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
    //printf("p %d\n",*a);
    int *b = new int();
    *b = *a - 1;
    //delete a;
    //printf("p %d\n",b);
    //printf("b %p\n",&b);
    if(*a > 0){
        thread_create((thread_startfunc_t) blueF, (void *) b);
    }
    thread_unlock(2);
}

int main() {
    int *a = new int();
    *a = 10;
    if (thread_libinit( (thread_startfunc_t) purpleF, (void *) a)) {
    cout << "thread_libinit failed\n";
    exit(1);
  }
}
