#include "thread.h"
#include <assert.h>
#include <stdio.h>    /* printf, fopen */
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <iostream>
#include <vector>

using namespace std;

//Structs
struct sandwich {
    int sandwich_type;
    int cashier_num;
};

struct cashier_obj {
    int number;
    int orders[3];
} cashier_obj_t;

std::vector<sandwich*> cork_board;

// Global Variables
int MAX_SIZE_CORK_BOARD;
int last_sandwich = -1;
int last_cashier = -1;
int thread_count = 0;
int cashierNumber = 0;


//Locks and Conditioned Variables
int BOARD_LOCK   = 1523;
int SPACE_SIGNAL = 9824;
int FULL_SIGNAL  = 2348;

void cashier_thread(void* cashier_info){
    cashier_obj* cashInfo = reinterpret_cast<cashier_obj *>(cashier_info);

    thread_lock(BOARD_LOCK);

    std::vector<int> input;
    //Iterate through cashier's sandwich list
    if(cashInfo->number == 0){
        input.push_back(290);
        input.push_back(858);
        input.push_back(254);

    }
    else if(cashInfo->number == 1){
        input.push_back(29);
        input.push_back(8);
        input.push_back(9);
    }
    else if(cashInfo->number == 2){
        input.push_back(223);
        input.push_back(568);
        input.push_back(274);

    }
    else if(cashInfo->number == 3){
        input.push_back(0);
        input.push_back(876);
        input.push_back(999);
    }
    else if(cashInfo->number == 4){
        input.push_back(980);
        input.push_back(86);
        input.push_back(244);
    }
    else{
        input.push_back(1);
        input.push_back(2);
        input.push_back(3);
    }
    int sandwich_num = 0;

    for (int i = 0; i < 3; i++){
        sandwich_num = input[i];
        //Fairness: if this cashier already went skip it
        while(cashInfo->number == last_cashier){
            thread_broadcast(BOARD_LOCK,SPACE_SIGNAL);
            thread_wait(BOARD_LOCK, SPACE_SIGNAL);
        }
        //If the cork board is full wait for the sandwich maker
        while(cork_board.size() == MAX_SIZE_CORK_BOARD){
            thread_signal(BOARD_LOCK,FULL_SIGNAL);
            thread_wait(BOARD_LOCK, SPACE_SIGNAL);
        }
        //Put something on the corkboard
        cout << "POSTED: cashier " << cashInfo->number << " sandwich " << sandwich_num << endl;
        sandwich* order = new sandwich();
        order->sandwich_type = sandwich_num;
        order->cashier_num = cashInfo->number;

        cork_board.push_back(order);

        last_cashier = cashInfo->number; // update laste_cashier
      }
    /*Close thread and signal the sandiwich maker. If this is the last thread, the sandwich maker can close everything,
      other wise the sandwich maker can just signal the next cashier */
    thread_count--;
    thread_signal(BOARD_LOCK, FULL_SIGNAL);
    thread_unlock(BOARD_LOCK);
}

void sandwich_maker(){
    thread_lock(BOARD_LOCK);

    //While the cork board isn't empty or there are more cashiers
    while (cork_board.size() != 0 || thread_count != 0){

        // Wait: while there is space on the board and there are still cashiers
        while(thread_count != 0 && cork_board.size() != MAX_SIZE_CORK_BOARD ){
          thread_broadcast(BOARD_LOCK, SPACE_SIGNAL);
          thread_wait(BOARD_LOCK, FULL_SIGNAL);
        }

        // Find next sandwich to make
        int smallest = 0;
        int j;
        for(j=0;j<cork_board.size();j++){
          if(std::abs(cork_board[j]->sandwich_type - last_sandwich) < std::abs(cork_board[smallest]->sandwich_type - last_sandwich)){
            smallest = j;
            }
        }

        //Make sandwich
        last_sandwich = cork_board[smallest]->sandwich_type;
        cout << "READY: cashier " << cork_board[smallest]->cashier_num << " sandwich " << cork_board[smallest]->sandwich_type << endl;
        cork_board.erase(cork_board.begin() + smallest);
      }
      thread_unlock(BOARD_LOCK);
}

void start_func(int inp) {
    int i;
    thread_count += cashierNumber;
    //Create sandwich maker
    thread_create((thread_startfunc_t) sandwich_maker, NULL);

    //Create the cashier threads with their files
    for(i = 0; i < cashierNumber; i++){
        cashier_obj*  obj =(cashier_obj*) malloc(sizeof(cashier_obj));
        obj->number = i;
        void* objpointer = (void *) obj;

        thread_create((thread_startfunc_t) cashier_thread, objpointer);
    }
}

int main(){
      cashierNumber = 6;
      MAX_SIZE_CORK_BOARD = 5;
      int x = 0;

      thread_libinit((thread_startfunc_t) start_func,(void *) &x);
      return 0;
}
