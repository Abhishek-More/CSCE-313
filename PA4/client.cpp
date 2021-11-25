#include "Histogram.h"
#include "Semaphore.h"
#include "common.h"
#include "FIFOreqchannel.h"
#include "BoundedBuffer.h"
#include "HistogramCollection.h"
#include <string>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <thread>
using namespace std;

struct resp {
  int patient;
  double value;
};

void patient_thread_function(int n, int patient, BoundedBuffer* buffer){
  //Pushes Requests onto the bounded buffer
  for(int i = 0; i < n; i++) {
    DataRequest req(patient, i*0.004, 1);
    vector<char> arr((char*)&req, ((char*) &req + sizeof(req)));
    buffer->push(arr);
  }
}

void patient_file_thread_function(BoundedBuffer* buffer, int m, FIFORequestChannel* chan,  string filename) {
    FileRequest fm (0,0);
    int len = sizeof (FileRequest) + filename.size()+1;
    char buf2 [len];
    cout << filename;
    memcpy (buf2, &fm, sizeof (FileRequest));
    strcpy (buf2 + sizeof (FileRequest), filename.c_str());
    chan->cwrite (buf2, len);  
    int64 filelen;
    chan->cread (&filelen, sizeof(int64));

    filename = "received/" + filename;
    FILE* file = fopen(filename.c_str(), "w");
    fseek(file, filelen, SEEK_SET);
    fclose(file);

    FileRequest* f = (FileRequest*) buf2;
    f->offset = 0;
    int64 rem = filelen;

    while (rem > 0) {
      f->length = (int) min(rem, (int64)m);
      vector<char> arr((char*)&buf2, ((char*) &buf2 + sizeof(buf2)));
      buffer->push(arr);
      rem -= f->length;
      f->offset += f->length;
    }

    Request quitter(QUIT_REQ_TYPE);
    chan->cwrite(&quitter, sizeof(quitter));

}

void worker_thread_function(FIFORequestChannel* chan, BoundedBuffer* buffer, BoundedBuffer* responseBuffer, HistogramCollection* hc, int m){

  double num = 0;

  while(1) {
    //Stackoverflow page showing how to cast vector<char> to char*
    //https://stackoverflow.com/questions/4254615/how-to-cast-vectorunsigned-char-to-char/4254644
    vector<char> temp = buffer->pop();
    char* request = reinterpret_cast<char*>(temp.data());

    Request r = *(Request*) request;
    
    if (r.getType() == DATA_REQ_TYPE) {

      chan->cwrite((char*) request, temp.size());
      chan->cread(&num, sizeof(double));
      DataRequest* dataRequest = (DataRequest*) request;

      resp response;
      response.patient = dataRequest->person;
      response.value = num;

      //hc->update(response.patient, response.value);
      vector<char> arr((char*)&response, ((char*) &response + sizeof(response)));
      responseBuffer->push(arr);

    } else if(r.getType() == FILE_REQ_TYPE) {

      FileRequest* fileRequest = (FileRequest*) request;
      char buf[m];
      string filename(fileRequest->getFileName());
      string fileDest = "received/" + filename; 
      chan->cwrite((char*) request, temp.size());
      chan->cread((char*)buf, m);
      
      FILE* file = fopen(fileDest.c_str(), "r+");
      fseek(file, fileRequest->offset, SEEK_SET);
      fwrite(buf, 1, fileRequest->length, file);
      fclose(file);

    } else {
        Request quitter(QUIT_REQ_TYPE);
        chan->cwrite(&quitter, sizeof(quitter));
        break;
    }
 }


}
void histogram_thread_function (BoundedBuffer* responseBuffer, HistogramCollection* hc){
  while(1) {

    vector<char> temp = responseBuffer->pop();
    resp* response = reinterpret_cast<resp*>(temp.data());

    if(response->patient == -1) {
      break;
    } else {
      hc->update(response->patient, response->value);
    }
  }
}

int main(int argc, char *argv[]){

    int opt;
    int p = 2;
    int w = 4;
    int h = 3;
    int n = 200;
    int m = MAX_MESSAGE;
    string filename = "";
    int b = 1024; // size of bounded buffer, note: this is different from another variable buffercapacity/m
    // take all the arguments first because some of these may go to the server
    while ((opt = getopt(argc, argv, "f:w:p:b:n:m:h:")) != -1) {
      switch (opt) {
        case 'f':
          filename = optarg;
          break;
        case 'w':
          w = atoi(optarg);
          break;
        case 'p':
          p = atoi(optarg);
          break;
        case 'h':
          h = atoi(optarg);
          break;
        case 'n':
          n = atoi(optarg);
          break;
        case 'b':
          b = atoi(optarg);
          break;
        case 'm':
          m = atoi(optarg);
          break;
      }
    }

    int pid = fork ();
    if (pid < 0){
      EXITONERROR ("Could not create a child process for running the server");
    }
    if (!pid){ // The server runs in the child process
      char* args[] = {"./server", nullptr};
      if (execvp(args[0], args) < 0){
        EXITONERROR ("Could not launch the server");
      }
    } 

      FIFORequestChannel* chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
      BoundedBuffer* request_buffer = new BoundedBuffer(b);
      BoundedBuffer* response_buffer = new BoundedBuffer(b);
      HistogramCollection hc;
      vector<thread> patients;
      vector<thread> workers;
      vector<thread> hThreads;

      for(int i = 0; i < p; i++) {
        Histogram* histogram = new Histogram(10, -2, 2);
        hc.add(histogram);
      }

      struct timeval start, end;
      gettimeofday (&start, 0);

      /* Start all threads here */
      if(filename.size() > 0) {

        Request newChan(NEWCHAN_REQ_TYPE);
        chan->cwrite(&newChan, sizeof(newChan));
        char name[1024];
        chan->cread(name, sizeof(name));
        FIFORequestChannel* newChannel = new FIFORequestChannel(name, FIFORequestChannel::CLIENT_SIDE);

        thread filet(patient_file_thread_function, request_buffer, m, newChannel, filename);

        for(int i = 0; i < w; i++) {
          Request nc(NEWCHAN_REQ_TYPE);
          chan->cwrite(&nc, sizeof(nc));
          char name[1024];
          chan->cread(name, sizeof(name));
          FIFORequestChannel* worker = new FIFORequestChannel(name, FIFORequestChannel::CLIENT_SIDE);
          workers.push_back(thread(worker_thread_function, worker, request_buffer, response_buffer, &hc, m));
        }

        filet.join();

        for(int i = 0; i < w; i++) {
          Request quitter(QUIT_REQ_TYPE);
          vector<char> req((char*)&quitter, ((char*) &quitter + sizeof(quitter)));
          request_buffer->push(req);
        }

        for(int i = 0; i < w; i++) {
          workers[i].join();
        }


      } else {

     
        for(int i = 0; i < p; i++) {
          patients.push_back(thread(patient_thread_function, n, i+1, request_buffer));
        }
        
        for(int i = 0; i < w; i++) {
          Request nc(NEWCHAN_REQ_TYPE);
          chan->cwrite(&nc, sizeof(nc));
          char name[1024];
          chan->cread(name, sizeof(name));
          FIFORequestChannel* worker = new FIFORequestChannel(name, FIFORequestChannel::CLIENT_SIDE);
          workers.push_back(thread(worker_thread_function, worker, request_buffer, response_buffer, &hc, m));
        }
        
        for(int i = 0; i < h; i++) {
          hThreads.push_back(thread(histogram_thread_function, response_buffer, &hc));
        }
       
        cout << "Starting" << endl;


        for(int i = 0; i < p; i++) {
          patients[i].join();
        }

        cout << "Patients done" << endl;

        for(int i = 0; i < w; i++) {
          Request quitter(QUIT_REQ_TYPE);
          vector<char> req((char*)&quitter, ((char*) &quitter + sizeof(quitter)));
          request_buffer->push(req);
        }

        for(int i = 0; i < w; i++) {
          workers[i].join();
        }

        for(int i = 0; i < h; i++) {
          resp stop;
          stop.patient = -1;
          stop.value = 0; 
          vector<char> req((char*)&stop, ((char*) &stop + sizeof(stop)));
          response_buffer->push(req);
        }

        for(int i = 0; i < h; i++) {
          hThreads[i].join();
        }
        
        hc.print();
      }
          /* Join all threads here */
      gettimeofday (&end, 0);

      // print the results and time difference
      //hc.print ();
      int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)/(int) 1e6;
      int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)%((int) 1e6);
      cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;
          
      Request q (QUIT_REQ_TYPE);
      chan->cwrite (&q, sizeof (Request));
      wait(0);
      cout << "Client process exited" << endl;

    return 0;
}
