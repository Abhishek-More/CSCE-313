#include <iostream>
#include <string.h>
#include <iomanip>
using namespace std;

#include "Histogram.h"

Histogram::~Histogram(){
}

vector<int> Histogram::get_hist(){
	return hist;
}

vector<double> Histogram::get_range (){
	vector<double> r;
	r.push_back (start);
	r.push_back (end);
	return r;
}
int Histogram::size(){
	return nbins;		
}
