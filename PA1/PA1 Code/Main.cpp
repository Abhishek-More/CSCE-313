#include "Ackerman.h"
#include "BuddyAllocator.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void easytest(BuddyAllocator* ba){
  // be creative here
  // know what to expect after every allocation/deallocation cycle

  // here are a few examples
  ba->printlist();
  // allocating a byte
  char * mem = ba->alloc (1);
  ba->printlist();
  ba->free (mem); // give back the memory you just allocated
  cout<<"Should be normal now";
  ba->printlist(); // shouldn't the list now look like as in the beginning

}

int main(int argc, char ** argv) {

  int basic_block_size = 128;
  int memory_length = 512 * 1024;
  //memory_length = 524288;

  int flags, opt;
  int tfnd = 0;

  while ((opt = getopt(argc, argv, "s:b:")) != -1) { 
    switch (opt) {
    case 's':
        memory_length = atoi(optarg);
        break;
    case 'b':
        basic_block_size = atoi(optarg);
        tfnd= 1;
        break;
  
    default: /* '?' */
        fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }
  }
  

  // create memory manager
  BuddyAllocator * allocator = new BuddyAllocator(basic_block_size, memory_length);

  // the following won't print anything until you start using FreeList and replace the "new" with your own implementation
  easytest (allocator);


  
  // stress-test the memory manager, do this only after you are done with small test cases
  Ackerman* am = new Ackerman ();
  am->test(allocator); // this is the full-fledged test. 
  
  // destroy memory manager
  delete allocator;

  return 0;
}
