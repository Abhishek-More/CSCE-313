#ifndef _BuddyAllocator_h_                   // include file only once
#define _BuddyAllocator_h_
#include <iostream>
#include <vector>
#include <math.h>
using namespace std;
typedef unsigned int uint;

/* declare types as you need */

class BlockHeader{
public:
	// think about what else should be included as member variables
	int block_size;  // size of the block
	BlockHeader* next; // pointer to the next block
        char isFree;

        BlockHeader(int _s = 0) {
          block_size = _s;
          next = nullptr;
          isFree = 1;
        }
};

class LinkedList{
	// this is a special linked list that is made out of BlockHeader s. 
public:
	BlockHeader* head;		// you need a head of the list
public:
        LinkedList(BlockHeader* h = nullptr) {
          this->head = h;
        }
	void insert (BlockHeader* b){	// adds a block to the list
          if(!b) {
            return;
          }

          if(!head) {
            head = b;
          } else {
            BlockHeader* temp = head;
            while(temp->next) {
              temp = temp->next;
            }
            temp->next = b;
          }
	}

	void remove (BlockHeader* b){  // removes a block from the list
          if(!head) {
            return;
          } else if(head == b) {
            BlockHeader* temp = head;
            head = head->next;
            //delete temp;
          } else {
            BlockHeader* temp = head;
            while(temp->next) {
              if(temp->next == b) {
                BlockHeader* deleter = temp->next;
                temp->next = temp->next->next;
                //delete deleter;
              } 
            }
          }
	}

        BlockHeader* remove() {
          if(head) {
            BlockHeader* temp = head;
            head = head->next;
            return temp; 
          }
          return nullptr;
        }
};


class BuddyAllocator{
private:
	/* declare more member variables as necessary */
	vector<LinkedList> FreeList;
	int basic_block_size;
	int total_memory_size;
        char* start;

private:
	/* private function you are required to implement
	 this will allow you and us to do unit test */
	 void printer(int index);
	
	BlockHeader* getbuddy (BlockHeader * addr); 


	// given a block address, this function returns the address of its buddy 

	bool arebuddies (BlockHeader* block1, BlockHeader* block2) {
          return getbuddy(block1) == block2;
        }
	// checks whether the two blocks are buddies are not

	BlockHeader* merge (BlockHeader* block1, BlockHeader* block2);
	// this function merges the two blocks returns the beginning address of the merged block
	// note that either block1 can be to the left of block2, or the other way around

	BlockHeader* split (BlockHeader* block);
	// splits the given block by putting a new header halfway through the block
	// also, the original header needs to be corrected
        //
        int getIndex(int size) {
          return log2(size/basic_block_size);
        }


public:
	BuddyAllocator (int _basic_block_size, int _total_memory_length); 
	/* This initializes the memory allocator and makes a portion of 
	   ’_total_memory_length’ bytes available. The allocator uses a ’_basic_block_size’ as 
	   its minimal unit of allocation. The function returns the amount of 
	   memory made available to the allocator. If an error occurred, 
	   it returns 0. 
	*/ 

	~BuddyAllocator(); 
	/* Destructor that returns any allocated memory back to the operating system. 
	   There should not be any memory leakage (i.e., memory staying allocated).
	*/ 

	char* alloc(int _length); 
	/* Allocate _length number of bytes of free memory and returns the 
		address of the allocated portion. Returns 0 when out of memory. */ 

	int free(char* _a); 
	/* Frees the section of physical memory previously allocated 
	   using ’my_malloc’. Returns 0 if everything ok. */ 
   
	void printlist ();
	/* Mainly used for debugging purposes and running short test cases */
	/* This function should print how many free blocks of each size belong to the allocator
	at that point. The output format should be the following (assuming basic block size = 128 bytes):

	[0] (128): 5
	[1] (256): 0
	[2] (512): 3
	[3] (1024): 0
	....
	....
	 which means that at this point, the allocator has 5 128 byte blocks, 3 512 byte blocks and so on.*/
};

#endif 
