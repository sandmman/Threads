#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>

/*Expected output:

*/
using namespace std;

void helpMeOut(void *i){
    thread_lock(5);

    cout << thread_signal(5, 2) << endl;
    cout << thread_wait(5, 2) << endl;

    thread_unlock(5);

}


void wreckMyLibFam(void *i){
    cout << thread_lock(1) << endl;
    cout << thread_lock(2) << endl;

    //Signal on nonexistant lock
    cout << thread_signal(3, 15) << endl;
    //Same CV, different locks
    cout << thread_signal(1, 10) << endl;
    cout << thread_signal(2, 10) << endl;

    cout << thread_unlock(2) << endl;

    cout << thread_wait(2, 20) << endl;

    thread_create(helpMeOut, NULL);

    cout << "Waiting..." << endl;
    thread_wait(1, 2);


    cout << "Back from wait." << endl;


    cout << thread_unlock(1) << endl;
}

int main() {
    cout << "Welcome" << endl;
    thread_libinit(wreckMyLibFam, NULL);
    return 0;
}
