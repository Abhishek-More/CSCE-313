#include "common.h"
#include "FIFOreqchannel.h"
#include <ios>
#include <sys/select.h>
#include <sys/wait.h>
#include <fstream>

using namespace std;

void logger(string s) {
  ofstream f;
  f.open("logger.txt", std::ios_base::app);
  f << s << endl;
  f.close();
}

int main(int argc, char *argv[]){

	int opt;
	int p = 1;
	double t = -0.1;
	int e = -1;
	string filename = "";
        int buffercapacity = MAX_MESSAGE;
        string bufstring = "";
        bool newChannel = false;
	// take all the arguments first because some of these may go to the server
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) {
            switch (opt) {
                case 'f':
                    filename = optarg;
                    break;
                case 'p':
                    p = atoi(optarg);
                    break;
                case 'e':
                    e = atoi(optarg);
                    break;
                case 't':
                    t = atof(optarg);
                    break;
                case 'm':
                    buffercapacity = atoi(optarg);
                    bufstring = optarg;
                    break;
                case 'c':
                    newChannel = true;
                    break;
            }
	}

	int pid = fork ();
	if (pid < 0){
		EXITONERROR ("Could not create a child process for running the server");
	}
        
	if (!pid){ // The server runs in the child process
          if(bufstring.size() != 0) {
              char* args[] = {"./server", "-m", (char*)bufstring.c_str(), nullptr};
              if (execvp(args[0], args) < 0){
                      EXITONERROR ("Could not launch the server");
              }
          } else {
              char* args[] = {"./server", nullptr};
              if (execvp(args[0], args) < 0){
                      EXITONERROR ("Could not launch the server");
              }
          }
	}

	FIFORequestChannel* chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
        FIFORequestChannel* temp = chan;
        //create new channel and make it the main channel
        if(newChannel) {
          Request nc(NEWCHAN_REQ_TYPE);
          chan->cwrite(&nc, sizeof(nc));
          char name[1024];
          chan->cread(name, sizeof(name));
          chan = new FIFORequestChannel(name, FIFORequestChannel::CLIENT_SIDE);
          cout<<"Using New Channel"<<endl;
        }

        //Trying to get 1 person info
        if(filename.size() == 0 && !newChannel) {
          if(t == -0.1 && e == -1) {
            //Getting 1000 values
            struct timeval time_now{};
            struct timeval time_end{};
            gettimeofday(&time_now, nullptr);
            ofstream file;
            double ecg1 = 0;
            double ecg2 = 0;
            file.open("received/x1.csv");
            for(int i = 0; i < 1000; i++) {
              DataRequest d (p, i*0.004, 1);
              chan->cwrite (&d, sizeof (DataRequest)); // question
              double reply;
              chan->cread (&reply, sizeof(double)); //answer
              if (!isValidResponse(&reply)){
                exit(0);
              }
              ecg1 = reply;

              DataRequest d2 (p, i*0.004, 2);
              chan->cwrite (&d2, sizeof (DataRequest)); // question
              double reply2;
              chan->cread (&reply2, sizeof(double)); //answer
              if (!isValidResponse(&reply2)){
                exit(0);
              }

              ecg2 = reply2;
              file << i*0.004 << "," << ecg1 << "," << ecg2;
              if(i != 999) {
                file << "\n";
              }
            }
             gettimeofday(&time_end, nullptr);
            cout<<"Time Taken: "<< time_end.tv_usec - time_now.tv_usec;
            file.close();

          } else {
            struct timeval time_now{};
            struct timeval time_end{};
            gettimeofday(&time_now, nullptr);
            DataRequest d (p, t, e);
            chan->cwrite (&d, sizeof (DataRequest)); // question
            double reply;
            chan->cread (&reply, sizeof(double)); //answer
            if (isValidResponse(&reply)){
              cout << "For person " << p <<", at time " << t << ", the value of ecg "<< e <<" is " << reply << endl;
            } else {
              exit(0);
            }
            gettimeofday(&time_end, nullptr);
            cout<<"Time Taken: "<< time_end.tv_usec - time_now.tv_usec;
          }
        }
	
        //File Transfer
        if(filename.size() > 0) {
          FileRequest fm (0,0);
          int len = sizeof (FileRequest) + filename.size()+1;
          char buf2 [len];
          memcpy (buf2, &fm, sizeof (FileRequest));
          strcpy (buf2 + sizeof (FileRequest), filename.c_str());
          chan->cwrite (buf2, len);  
          int64 filelen;
          chan->cread (&filelen, sizeof(int64));
          if (isValidResponse(&filelen)){
                  cout << "File length is: " << filelen << " bytes" << endl;
          }
          
          FileRequest* f = (FileRequest*) buf2;
          f->offset = 0;
          int64 rem = filelen;
          int length = 0;
          ofstream of("received/" + filename);
          
          char* recvbuf = new char[buffercapacity];
          while (rem > 0) {
            f->length = (int) min(rem, (int64)buffercapacity);
            chan->cwrite(buf2, len);
            int res = chan->cread((char*)recvbuf, buffercapacity);
            of.write((char*)recvbuf, f->length);
            rem -= f->length;
            f->offset += f->length;
          }
          of.close();

        }
        
       	// closing the channel    
        Request q (QUIT_REQ_TYPE);
        chan->cwrite (&q, sizeof (Request));
        if(newChannel) {
          //Quit Original Channel;
          Request quitTemp (QUIT_REQ_TYPE);
          temp->cwrite (&q, sizeof (Request));
          wait(0);
        }
	// client waiting for the server process, which is the child, to terminate

        logger("HI");
	wait(0);
	cout << "Client process exited" << endl;
}
