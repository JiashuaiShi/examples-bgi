#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

class MyQueue {
private:
    mutex mut;
    condition_variable cond;
    queue<string> q;

public:
    MyQueue() = default;
    MyQueue(const MyQueue &) = delete;
    MyQueue &operator=(const MyQueue &) = delete;

    void push(string &e) {
        std::lock_guard<std::mutex> lk(mut);
        q.push(e);
        cond.notify_one();
    }

    string pop() {
        lock_guard<mutex> lk(mut);
        string e = move(q.front());
        q.pop();
        return e;
    }

    unsigned long size() {
        lock_guard<mutex> lk(mut);
        return q.size();
    }

    bool empty() {
        lock_guard<mutex> lk(mut);
        return q.empty();
    }
};

