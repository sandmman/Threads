#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <stdio.h>
/*Expected output:


*/
using namespace std;

int g=0;

void purpleF() {
    thread_lock(2);
    printf("Iteration: %d\n",g);
    g++;
    thread_create((thread_startfunc_t) purpleF, NULL);
    thread_unlock(2);
}

int main() {
    if (thread_libinit( (thread_startfunc_t) purpleF, NULL)) {
    cout << "thread_libinit failed\n";
    exit(0);
  }
}
