#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>

/*Expected output:

    parent called with arg 100
    loop called with id parent thread
    	parent thread:	0	0
    loop called with id child thread
    	child thread:	0	0
    	parent thread:	1	1
    	child thread:	1	2
    	parent thread:	2	3
    	child thread:	2	4
    	parent thread:	3	5
    	child thread:	3	6
    	parent thread:	4	7
    	child thread:	4	8
    Thread library exiting.

*/
using namespace std;

int g=0;

void loop(void *a) {
  char *id;
  int i;

  id = (char *) a;
  cout <<"loop called with id " << (char *) id << endl;

  for (i=0; i<5; i++, g++) {
    cout << "\t" << id << ":\t" << i << "\t" << g << endl;
    if (thread_yield()) {
      cout << "thread_yield failed\n";
      exit(1);
    }
  }
}

void parent(void *a) {
  int arg;
  arg = (long int) a;

  cout << "parent called with arg " << arg << endl;
  if (thread_create((thread_startfunc_t) loop, (void *) "child thread")) {
    cout << "thread_create failed\n";
    exit(1);
  }

  loop( (void *) "parent thread");
}

int main() {
  if (thread_libinit( (thread_startfunc_t) parent, (void *) 100)) {
    cout << "thread_libinit failed\n";
    exit(1);
  }
}
