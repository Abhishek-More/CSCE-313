#include <thread>
#include <vector>

#include "Semaphore.h"

Semaphore A(0);
Semaphore B(0);
Semaphore C(1);
Semaphore l(1);
int bRuns = 0;

void a_thread_function() {
  while(true){ 
    C.P(); //Check if C is done
    //Run Thread A Operation

    A.V(); //Allow B to run twice
    A.V();
  }
}

void b_thread_function() {
  while(true) {
    A.P(); //Check if A is done
    l.P(); //Lock so both B Threads dont cause race condition
    //Run Thread B Operation
    bRuns++;
    
    if(bRuns == 2) {  //Make sure B only runs twice
      bRuns = 0;
      B.V(); //Let C know that B is done
    }

    l.V(); //Allow next B thread to run
    
  }
}

void c_thread_function() {
  while(true) {
    B.P();
    //Run Thread C Operation
    C.V(); //Let Thread A know that C is done
  }
}

