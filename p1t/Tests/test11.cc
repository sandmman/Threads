#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>

/*Expected output:
Welcome
-1
-1
-1
-1
-1
-1
-1
-1
0
Locked
-1
0
-1
Thread library exiting.

*/
using namespace std;

void wreckMyLibFam(void *i){
    cout << thread_libinit(wreckMyLibFam, NULL) << endl;
    cout << thread_lock(1) << endl;
    cout << "Locked" << endl;
    cout << thread_lock(1) << endl;
    cout << thread_unlock(1) << endl;
    cout << thread_unlock(1) << endl;
}

int main() {
    cout << "Welcome" << endl;
    cout << thread_create(wreckMyLibFam, NULL) << endl;
    cout << thread_yield() << endl;
    cout << thread_lock(1) << endl;
    cout << thread_unlock(1) << endl;
    cout << thread_wait(1, 2) << endl;
    cout << thread_signal(1, 2) << endl;
    cout << thread_broadcast(1, 2) << endl;


    thread_libinit(wreckMyLibFam, NULL);
    return 0;
}
