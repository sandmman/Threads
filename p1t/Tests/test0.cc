#include "thread.h"
#include <stdio.h>


/*Expected output:
    7
    14
    Thread library exiting.
*/
void not_basic(int *i){
    printf("%d\n", (*i)*2);
}

void basic(int *i){
    thread_create((thread_startfunc_t) not_basic, (void *) i);
    printf("Made not basic\n");
    printf("%d\n", *i);
}

int main(int argc,char* argv[]){
    int i = 7;
    printf("a\n");
    thread_libinit((thread_startfunc_t) basic, (void *) &i);
    printf("Done initializing\n");
}
