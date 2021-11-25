#ifndef BoundedBuffer_h
#define BoundedBuffer_h

#include <stdio.h>
#include <queue>
#include <string>
#include "Semaphore.h"

using namespace std;

class BoundedBuffer
{
private:
	int cap; // max number of items in the buffer
	queue<vector<char>> q;

	// add necessary synchronization variables (e.g., sempahores, mutexes) and variables
        Semaphore* mut;
        Semaphore* full;
        Semaphore* empty;


public:
	BoundedBuffer(int cap){
          this->cap = cap;
          
          //Init binary semaphore 
          mut = new Semaphore(1);

          //init full / empty semaphores
          full = new Semaphore(0);
          empty = new Semaphore(this->cap);
	}
	~BoundedBuffer(){

	}

	void push(vector<char> data){
          
          empty->P();
          mut->P();
          q.push(data);
          mut->V();
          full->V();
		
	}

	vector<char> pop(){
          //1. Wait using the correct sync variables 
          //2. Pop the front item of the queue. 
          //3. Unlock and notify using the right sync variables
          //4. Return the popped vector

          full->P();
          mut->P();
          vector<char> res = q.front();
          q.pop();
          mut->V();
          empty->V();
          
          return res;
	}
};

#endif /* BoundedBuffer_ */
