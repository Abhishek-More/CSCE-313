#include <condition_variable>
#include <mutex> 
#include <vector> 
#include <queue> 

using namespace std;

class BoundedBuffer {
  private:
    int cap;
    queue<vector<char>> q;
    condition_variable dataPresent;
    condition_variable emptySlot;
    mutex m;
    double percent;

  public:
    BoundedBuffer(int cap) {
      this->cap = cap;
      this->percent = 0.1;
    }


    ~BoundedBuffer() {

    }

    void push(vector<char> data) {
      m.lock();
      this->q.push(data);
      m.unlock();
      dataPresent.notify_all();
    }

    vector<char> pop() {
      unique_lock<mutex> lock(m);
      dataPresent.wait(lock, [this]{return q.size() > cap * this->percent;});
      vector<char> res = q.front();
      q.pop();
      emptySlot.notify_one();
      lock.unlock();
      return res;
    }


};
