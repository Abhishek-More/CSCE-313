#include "BuddyAllocator.h"
#include <cstdlib>
#include <iostream>
#include <math.h>
#include <cmath>
using namespace std;

BlockHeader* BuddyAllocator::split(BlockHeader* b) {
  int bs = b->block_size;
  b->block_size = b->block_size/2;
  b->next = nullptr;
  BlockHeader* sh =(BlockHeader*) ((char*) b + b->block_size);
  //sh->next = nullptr;
  //sh->block_size = b->block_size;
  BlockHeader* temp = new (sh) BlockHeader(b->block_size);
  return sh;

}

BuddyAllocator::BuddyAllocator (int _basic_block_size, int _total_memory_length){
  total_memory_size = _total_memory_length;
  basic_block_size = _basic_block_size;

  start = new char[total_memory_size];
  int l = log2(total_memory_size/basic_block_size);

  for(int i = 0; i < l; i++) {
    FreeList.push_back(LinkedList());
  }

  FreeList.push_back(LinkedList((BlockHeader*)start));
  BlockHeader* header = new (start) BlockHeader (total_memory_size);

}

BuddyAllocator::~BuddyAllocator (){
  delete [] start;
}

char* BuddyAllocator::alloc(int length) {
  
  //find smallest block size
  //use freelist to find available block of sbs or larger
  //If block is larger than sbs, call split until we reach sbs
  //Once sbs blcoks are available on the freelist, remove one from the free list
  //Shift the address down sizeof(blockheader) bytes and return it.

  int x = length + sizeof(BlockHeader);
  int calc = ceil ((float) x / basic_block_size);
  int index = ceil((float) log2 (calc));
  int blockSizeReturn = (1 << index) * basic_block_size;
  
  if(FreeList[index].head){
    BlockHeader* h =  FreeList[index].remove();
    h->isFree = 0;
    return (char*) (h+1);
  }

  int orig = index;
  for(; index < FreeList.size(); index++) {
    if(FreeList[index].head) {
      break;
    }
  }

  if(index >= FreeList.size()){
    return nullptr;
  }

  while(index > orig) {
    BlockHeader* b = FreeList[index].remove();
    BlockHeader* temp = split(b);
    index--;
    FreeList[index].insert(b);
    FreeList[index].insert(temp);
  } 
  BlockHeader* block = FreeList[index].remove();
  block->isFree = 0;
  return (char*)(block+1);
}

int BuddyAllocator::free(char* _a) {
  BlockHeader* temp = (BlockHeader*)(_a - sizeof(BlockHeader));
  while(true) {
    int size = temp->block_size;
    temp->isFree = 1;
    int index = getIndex(size);

    if(index == FreeList.size()-1) {
      FreeList[index].insert(temp);
      break;
    }
    BlockHeader* buddy = getbuddy(temp);
    if(buddy->isFree == 1 && temp->block_size == buddy->block_size){
      FreeList[index].remove(buddy);
      temp = merge(temp, buddy);
    } else {
      FreeList[index].insert(temp);
      break;
    }
  }
  return 0;
}

BlockHeader* BuddyAllocator::getbuddy(BlockHeader* addr) {
  //return (BlockHeader*) ((int)((char*)addr - start) ^ addr->block_size + start);
  return (BlockHeader*) (((int)((char*) addr - start) ^ addr->block_size) + start);
  
}

BlockHeader* BuddyAllocator::merge(BlockHeader* sm, BlockHeader* big) {

  if(big < sm) {
    BlockHeader* temp = sm;
    sm = big;
    big = temp;
  }
  
  int index = getIndex(sm->block_size);
  FreeList[index].remove(sm);
  sm->block_size *= 2;
  sm->next = nullptr;
  //index = getIndex(smSize);
  //FreeList[index].insert(sm);
  return sm;
}

void BuddyAllocator::printlist (){
  cout << "Printing the Freelist in the format \"[index] (block size) : # of blocks\"" << endl;
  for (int i=0; i<FreeList.size(); i++){
    cout << "[" << i <<"] (" << ((1<<i) * basic_block_size) << ") : ";  // block size at index should always be 2^i * bbs
    int count = 0;
    BlockHeader* b = FreeList [i].head;
    // go through the list from head to tail and count
    while (b){
      count ++;
      // block size at index should always be 2^i * bbs
      // checking to make sure that the block is not out of place
      //cout<<b->block_size << " ";
      if (b->block_size != (1<<i) * basic_block_size){
        cerr << "ERROR:: Block is in a wrong list" << endl;
        exit (-1);
      }
      b = b->next;
    }
    cout << count << endl;  
  }
}

void BuddyAllocator::printer(int index) {
  BlockHeader* temp = FreeList[index].head; 

  while(temp) {
    cout<< temp->block_size<<" ";
    temp = temp->next;
  }

  std::cout<<endl; 
  
}

