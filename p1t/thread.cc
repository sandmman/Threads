#define _XOPEN_SOURCE 1
#include "thread.h"
#include <queue>
#include <map>
#include <assert.h>
#include <iostream>
#include <ucontext.h>
#include <cstdlib>
#include <stdio.h>
#include "interrupt.h"

using namespace std;

//This should never be greater than 1.
int thread_libinit_calls = 0;

//This is used to keep track of our garbage u_context pointers.
struct node {
    ucontext_t *u;
    node *next;
};


struct lockCV{
    int lockNum, condition;

    bool operator==(const lockCV &o) const{
        return lockNum == o.lockNum && condition == o.condition;
    }
    bool operator<(const lockCV &o) const{
        /*
        cout << "Compairing:" << endl;
        cout << "Lock: " << lockNum << "\tCond: " << condition << endl;
        cout << "Lock: " << o.lockNum << "\tCond: " << o.condition << endl;
        cout << (condition < o.condition || (condition == o.condition && lockNum < o.lockNum)) << endl;
        */
        return condition < o.condition || (condition == o.condition && lockNum < o.lockNum);
    }
};

void printPair(lockCV p){
    cout << "Lock: " << p.lockNum << "\tCond: " << p.condition << endl;
}

//The currently running thread
ucontext_t* running;

//The queue of threads ready to run
std::queue<ucontext_t*> ready;

//Locks map to ucontext pointers of the thread that holds that lock
std::map<int, ucontext_t*> heldLocks;

//Maps of locks and conditions and queues of threads waiting on them.
std::map<int, std::queue <ucontext_t*> > locked;
std::map<int, int> amir;
std::map<lockCV, std::queue <ucontext_t*> > waiting;

//Linked list of ucontext of exited threads that need to be freed.
node* garbage;

//Swaps contexts of the prev thread and the next the next thread to run.
int swapThreads(ucontext_t *prev, ucontext_t *next){
    running = next;
    swapcontext(prev, next);
    interrupt_enable();
    return 0;
}

//Frees stacks and ucontexts of things in the garbage linked list.
void basurero(){
    while(garbage != NULL){
        //Delete stack
        delete (char *) garbage -> u -> uc_stack.ss_sp;

        //Delete ucontext
        delete garbage->u;

        //Take the first node of garbage off
        node* tmp = garbage;
        garbage = garbage->next;

        //Free that node
        delete tmp;
        }
}

void start(thread_startfunc_t func, void *arg){
    //Enable interrupts right after this thread is created.
    interrupt_enable();

    //Preform the thread's function.
    func(arg);

    //Take the thread out of running and mark it for garbage collection.
    interrupt_disable();
    node *tmp;
    try{
        tmp = new node();
    }
    catch(std::bad_alloc& exc){
        basurero();
        tmp = new node();
    }

    tmp->u = running;
    tmp->next = garbage;
    garbage = tmp;

    //If the thread isn't the last one in the ready queue, run the next one.
    if (ready.size() > 0){
        running = ready.front();
        ready.pop();
        setcontext(running);
    }

    //If the thread is the last one in the ready queue, exit our library
    thread_libinit_calls--;
    cout << "Thread library exiting." << endl;
    interrupt_enable();
    exit(0);
}

int thread_create(thread_startfunc_t func, void *arg){
    interrupt_disable();
    if(thread_libinit_calls != 1){
        interrupt_enable();
        return -1;
    }

    //Collect the garbage.
    basurero();

    ucontext_t *ucontext_ptr;
    try {
        ucontext_ptr = new ucontext_t();
    }
    catch(std::bad_alloc& exc) {
        delete ucontext_ptr;
        interrupt_enable();
        return -1;
    }
    getcontext(ucontext_ptr); // ucontext_ptr has type (ucontext_t *)

    /*
    Direct the new thread to use a different stack.
    Your thread library should allocate STACK_SIZE bytes for each thread's stack.
    */
    char *stack;
    try{
        stack = new char [STACK_SIZE];
    }
    catch(std::bad_alloc& exc){
        delete stack;
        interrupt_enable();
        return -1;
    }
    ucontext_ptr->uc_stack.ss_sp = stack;
    ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
    ucontext_ptr->uc_stack.ss_flags = 0;
    ucontext_ptr->uc_link = NULL;
    //Direct the new thread to start by calling start(arg1, arg2)
    makecontext(ucontext_ptr, (void (*)()) start, 2 , func, arg);
    ready.push(ucontext_ptr);

    interrupt_enable();
    return 0; // return 0 on success. -1 on failure
}

int thread_libinit(thread_startfunc_t func, void *arg){
    interrupt_disable();

    thread_libinit_calls++;
    if(thread_libinit_calls != 1){
        thread_libinit_calls--;
        interrupt_enable();
        return -1;
    }

    interrupt_enable();
    thread_create(func, arg);
    interrupt_disable();

    assert(ready.size() >= 1);
    running = ready.front();
    ready.pop();

    //interrupt_enable();
    setcontext(running);
}
// Should swap contexts with the thread that is first in the ready state queue
int thread_yield(void){
    interrupt_disable();
    if(thread_libinit_calls != 1){
        interrupt_enable();
        return -1;
    }

    //There are no ready threads, we can't yield.
    if (ready.size() == 0){
        interrupt_enable();
        return -1;
    }

    //Next thread to run = nThread.
    ucontext_t *nThread = ready.front();

    //Take first of ready off ready; put running on the back of ready.
    ready.pop();
    ready.push(running);

    //swapThreads takes care of interrupt_enable.
    swapThreads(running, nThread);

    //Return 0 on success, -1 on failure.
    return 0;
}

int thread_lock(unsigned int lock){
    interrupt_disable();
    if(thread_libinit_calls != 1){
        interrupt_enable();
        return -1;
    }

    //If the lock hasn't been used before, initialize it's locked queue.
    if(locked.count(lock) == 0){
        std::queue <ucontext_t*>* q;

        basurero();

        try {
            q = new std::queue <ucontext_t*>();
            locked.insert(std::make_pair(lock, *q));
        }
        //If we couldn't allocate enough memory, show that we messed up.
        catch(std::bad_alloc& exc) {
            delete q;
            interrupt_enable();
            cout << "Thread library exiting." << endl;
            exit(0);
        }
    }

    //If no one holds the lock, aquire it, return, and keep running.
    if(heldLocks.count(lock) == 0){
        heldLocks[lock] = running;
        interrupt_enable();
        return 0;

    //If someone holds the lock
    } else {
        //Make sure that someone isn't us.(User error)
        if(running == heldLocks[lock]){
            interrupt_enable();
            return -1;
        }

        //Put the current thread on the locked queue.
        locked[lock].push(running);
        ucontext_t* temp = ready.front();
        ready.pop();

        //Deadlock happened, so exit.
        if (temp == NULL){
            cout << "Thread library exiting." << endl;
            interrupt_enable();
            exit(0);
        }

        //swapThreads takes care of interrupt_enable.
        swapThreads(running, temp);
        return 0;
    }
}

/*
A thread calls thread_unlock, the caller does not yield the CPU.
The woken thread is put on the ready queue but is not executed right away.
*/
int thread_unlock(unsigned int lock){
    interrupt_disable();
    if(thread_libinit_calls != 1){
        interrupt_enable();
        return -1;
    }

    //This lock doesn't actually exist or you don't own it. (User error)
    if(heldLocks.count(lock) == 0 ||  heldLocks[lock] != running){
        interrupt_enable();
        return -1;
    }

    //If you have the lock, release it.
    else{
        //Release the lock.
        heldLocks.erase(lock);

        /*
        If someone is waiting for the lock, give it to them and put them on the
        ready queue.
        */
        if(locked[lock].size() > 0){
            ucontext_t *temp = locked[lock].front();
            locked[lock].pop();
            ready.push(temp);
            heldLocks[lock] = temp;
        }
    }
    interrupt_enable();
    //Returns 0 on success, -1 on failure.
    return 0;
}

int thread_wait(unsigned int lock, unsigned int cond){
    interrupt_disable();
    if(thread_libinit_calls != 1){
        interrupt_enable();
        return -1;
    }

    //First, relinquish your lock.

    //If the lock doesn't exist, you're so fucked up. (User error)
    if(heldLocks.count(lock) == 0){
        interrupt_enable();
        return -1;
    } else {
        //Make sure the lock you're releasing is one you hold.
        if(running != heldLocks[lock]){
            interrupt_enable();
            return -1;
        }

        //Release the lock.
        heldLocks.erase(lock);

        /*
        If someone is waiting for the lock, give it to them and put them on the
        ready queue.
        */
        if(locked[lock].size() > 0){
            ready.push(locked[lock].front());
            heldLocks[lock] = locked[lock].front();
            locked[lock].pop();
        }


        if (amir.count(cond) != 0){
            if (amir[cond] != lock){
                interrupt_enable();
                return -1;
            }
        } else {
            amir[cond] = lock;
        }


        //Second, add yourself to the waiting queue.

        lockCV lc = {};
        lc.lockNum = lock;
        lc.condition = cond;

/*
        printf("Wait on:\n");
        printPair(lc);
        printf("%d\n", waiting.count(lc));
*/

        //This signal's waiting queue doesn't exist yet.
        if(waiting.count(lc) == 0){
            std::queue <ucontext_t*> *q;
            basurero();
            try {
                //Try to allocate a new waiting queue.
                 q = new std::queue <ucontext_t*>();
                waiting.insert(std::make_pair(lc, *q));
            }
            //If we couldn't allocate enough memory, show that we messed up.
            catch(std::bad_alloc& exc) {
                delete q;
                interrupt_enable();
                cout << "Thread library exiting." << endl;
                exit(0);
            }
        }

        //Put yourself on the waiting queue.
        waiting[lc].push(running);

        //Otherwise, swap to the first ready thread.
        ucontext_t *nThread = ready.front();
        ready.pop();

        //If deadlock happened, exit.
        if (nThread == NULL){
            cout << "Thread library exiting." << endl;
            interrupt_enable();
            exit(0);
        }

        /*
        swapThreads takes care of interrupt_enable, but the thread has to
        reaquire the lock it previously held before continuing execution.
        */
        swapThreads(running, nThread);
        thread_lock(lock);
        return 0;
    }
}

/*
When a thread calls thread_signal or thread_broadcast, the caller does not
yield the CPU. The woken thread is put on the ready queue. The woken thread
requests the lock when it next runs and returns from wait.
*/
int thread_signal(unsigned int lock, unsigned int cond){
    interrupt_disable();
    if(thread_libinit_calls != 1){
        interrupt_enable();
        return -1;
    }

    lockCV lc = {};
    lc.lockNum = lock;
    lc.condition = cond;

/*
    printf("Signal on:\n");
    printPair(lc);
    printf("%d\n", waiting.count(lc));
*/

    //This waiting queue hasn't been used yet. (User error?)
    if(waiting.count(lc) == 0){
        interrupt_enable();
        return 0;
    }

    //If there is someone waiting on this signal,
    if(waiting[lc].size() > 0){
        //Put them on ready queue,
        ready.push(waiting[lc].front());

        //And take them off the waiting queue.
        waiting[lc].pop();
    }
    interrupt_enable();
    return 0;
}

/*
When a thread calls thread_signal or thread_broadcast, the caller does not
yield the CPU. The woken thread is put on the ready queue but is not executed
right away. The woken threadrequests the lock when it next runs
*/
int thread_broadcast(unsigned int lock, unsigned int cond){
    interrupt_disable();
    if(thread_libinit_calls != 1){
        interrupt_enable();
        return -1;
    }

    lockCV lc = {};
    lc.lockNum = lock;
    lc.condition = cond;

/*
    printf("Broadcast on:\n");
    printPair(lc);
    printf("%d\n", waiting.count(lc));
*/

    //This waiting queue hasn't been used yet. (User error?)
    if(waiting.count(lc) == 0){
        interrupt_enable();
        return 0;
    }

    //While there are people waiting on this signal,
    while(waiting[lc].size() != 0){
        //Put them on the ready queue,
        ready.push(waiting[lc].front());

        //And take them off the waiting queue.
        waiting[lc].pop();
    }
    interrupt_enable();
    return 0;
}
