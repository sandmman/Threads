#include "thread.h"
#include <stdio.h>    /* printf, fopen */
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <iostream>
#include <vector>
#include <cmath>
//Comment//

using namespace std;

// Global Variables
struct sandwich {
  int sandwich_type;
  int cashier_num;
};

struct cashier_obj {
  int number;
  char* filename;
} cashier_obj_t;

std::vector<sandwich*> cork_board;

int MAX_SIZE_CORK_BOARD;
int last_sandwich = -1;
int last_cashier = -1;
int thread_count = 0;
int cashierNumber = 0;

int BOARD_LOCK   = 1523;
int SPACE_SIGNAL = 9824;
int FULL_SIGNAL  = 2348;

void printCorkBoard(){
  int i;
  for (i = 0; i < cork_board.size(); i++){
    printf("%d\t", cork_board[i]->sandwich_type);
  }
  printf("\n");
}


void cashier_thread(void* cashier_info){
  cashier_obj* cashInfo = reinterpret_cast<cashier_obj *>(cashier_info);
  ifstream in(cashInfo->filename);
  thread_lock(BOARD_LOCK);

  int sandwich_num;
  char str[255];
  while(!in.eof()){
    in.getline(str,255);
    sandwich_num = atoi(str);
    if(strlen(str) == 0){
      break;
    }
    //Fairness: if this board already went skip it
    while(cashInfo->number == last_cashier){
      thread_signal(BOARD_LOCK,SPACE_SIGNAL);
      thread_wait(BOARD_LOCK, SPACE_SIGNAL);
    }
    //If the cork board is full wait for maker
    while(cork_board.size() == MAX_SIZE_CORK_BOARD){
        thread_signal(BOARD_LOCK,FULL_SIGNAL);
        thread_wait(BOARD_LOCK, SPACE_SIGNAL);
    }
    //put something on the corkboard
    cout << "POSTED: cashier " << cashInfo->number << " sandwich " << sandwich_num << endl;
    sandwich* order = new sandwich();
    order->sandwich_type = sandwich_num;
    order->cashier_num = cashInfo->number;

    cork_board.push_back(order);
    last_cashier = cashInfo->number;
  }
  thread_count--;
  thread_signal(BOARD_LOCK, SPACE_SIGNAL);
  thread_unlock(BOARD_LOCK);
}


void sandwich_maker(){
  int last_sandwich = -1;
  thread_lock(BOARD_LOCK);

  while (cork_board.size() != 0){

    // Wait, while the board is not full and their are still cashiers
    while(thread_count != 0 && cork_board.size() != MAX_SIZE_CORK_BOARD ){
      thread_signal(BOARD_LOCK, SPACE_SIGNAL);
      thread_wait(BOARD_LOCK, SPACE_SIGNAL);
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

    // boradcast/wait
    thread_broadcast(BOARD_LOCK,SPACE_SIGNAL);
    thread_wait(BOARD_LOCK, SPACE_SIGNAL);

  }
  thread_unlock(BOARD_LOCK);
}
void start_func(char* argv[]) {
  thread_create((thread_startfunc_t) sandwich_maker, NULL);
  int i;
  for(i = 0; i < cashierNumber; i++){
  	//printf("%s\n", argv[i+2]);

    cashier_obj*  obj =(cashier_obj*) malloc(sizeof(cashier_obj));
    obj->number = i;
    obj->filename = argv[i+2];

  	void* objpointer = (void *) obj;

    thread_count++;
    thread_create((thread_startfunc_t) cashier_thread, objpointer);
  }
}

int main(int argc,char* argv[]){
  if(argc < 2){
    printf("Need more Files\n");
    exit(0);
  }
  else {
    cashierNumber = argc - 2;
    MAX_SIZE_CORK_BOARD = atoi(argv[1]);
    string* input_files = new string[argc-1];
    thread_libinit((thread_startfunc_t) start_func,(void *) argv);
  }
  return 0;
}
/*The first argument specifies the maximum number of orders that the cork board can hold.
 The rest of the arguments specify a list of input files (one input file per cashier).
 I.e., the input file for cashier c is argv[c+2], where 0 <= c < (number of cashiers).
 The number of cashier threads should be deduced from the number of input files specified.*/
