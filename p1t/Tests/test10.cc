#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>

/*Expected output:

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
    thread_libinit(wreckMyLibFam, NULL);
    return 0;
}
