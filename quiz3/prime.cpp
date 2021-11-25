#include <iostream>
#include <sys/signal.h>

using namespace std;

int num_primes = 0; // holds # of primes

void signal_handler(int signo) {
  cout<<"Partial Count"<<num_primes<<"\n";
  exit(0);
}

void count_primes (int start, int end){
    // check each number in range [start, end] for primality
  for (int num = start; num <= end; ++num) {
      int i;
      for (i = 2; (i <= num) && (num % i != 0); ++i)
          ;
      if (i == num)  // if no divisor found, it is a prime
          ++num_primes;
  }
}
int main (int ac, char** av){
  signal(SIGINT, signal_handler);
  count_primes (1, 1000000); // count all primes <= 1 million
  cout <<"Found "<<num_primes<<" prime numbers in the range"<<endl;
}

