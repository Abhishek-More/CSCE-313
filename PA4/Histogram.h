#ifndef Histogram_h
#define Histogram_h

#include <queue>
#include <string>
#include <vector>
#include <unordered_map>
#include <pthread.h>

#include "Semaphore.h"

using namespace std;

class Histogram {
private:
	vector<int> hist;
	int nbins;
	double start, end;
        Semaphore* s;
public:
    Histogram(int _nbins, double _start, double _end): nbins (_nbins), start(_start), end(_end){
            //memset (hist, 0, nbins * sizeof (int));	
            hist = vector<int> (nbins, 0);
            this->s = new Semaphore(1);
    }
    void update (double value){

            int bin_index = (int) ((value - start) / (end - start) * nbins);
            if (bin_index <0)
                    bin_index= 0;
            else if (bin_index >= nbins)
                    bin_index = nbins-1;

            //cout << value << "-" << bin_index << endl;
            //Lock and unlock this line

            s->P();
            hist [bin_index] ++;
            s->V();

    }
    ~Histogram();
    vector<int> get_hist();		// prints the histogram
    int size ();
    vector<double> get_range ();
};

#endif 
