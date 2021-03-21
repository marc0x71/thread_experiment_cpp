#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <chrono>
#include <vector>
#include <algorithm>
#include <deque>
#include <condition_variable>
#include <utility>

template <class T>
class MessageQueue {
public:
    MessageQueue() : _done(false) {}
    void put(const T& message) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _queue.push_back(message);
        }
        //_cond.notify_one();
    }
    T get() {
        std::unique_lock<std::mutex> lock(_mutex);
        //_cond.wait(lock, [&]() {return !_queue.empty(); });
        if (_queue.empty()) return "";
        T message = _queue.front();
        _queue.pop_front();
        return message;
    }
    bool done() {
        std::unique_lock<std::mutex> lock(_mutex);
        return _done;
    }
    bool active() {
        std::unique_lock<std::mutex> lock(_mutex);
        return !_queue.empty();
    }
    void terminate() {
        std::unique_lock<std::mutex> lock(_mutex);
        _done = true;
    }
private:
    std::deque<T> _queue;
    std::condition_variable _cond;
    std::mutex _mutex;
    bool _done;
};
typedef MessageQueue<std::string> MsgQueue;

void producer(MsgQueue& queue, int size) {
    char s[20];
    auto start = std::chrono::system_clock::now();
    for (int i = 0; i < size; i++) {
        sprintf(s, "message #%4.4d", i);
        queue.put(s);
        //std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    queue.terminate();
    auto end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    printf("PRODUCER-> elapsed %ld ms\n", elapsed);
}

void consumer(int num, MsgQueue& queue) {
    for (;;) {
        if (queue.active()) break;
    }
    printf("THREAD #%2.2d -> starting!\n", num);
    auto start = std::chrono::system_clock::now();
    int count = 0;
    int fault = 0;
    for (;;) {
        std::string message = queue.get();
        if (message.empty() && queue.done()) break;
        if (!message.empty()) {
            count++;
        } else {
            fault++;
        }
        //printf("THREAD #%2.2d -> got <%s>\n", num, message.c_str());
    }
    auto end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    printf("THREAD #%2.2d -> <%d/%d> elapsed %ld ms\n", num, count, fault, elapsed);
}

int main() {
    MsgQueue queue;

    std::cout << "start consumer..." << std::endl;
    std::vector<std::future<void> > futures;
    for (int i = 0; i < 10; i++) {
        auto fut = std::async(&consumer, i + 1, std::ref(queue));
        futures.push_back(std::move(fut));
    }
    std::cout << "start producer..." << std::endl;
    auto fut = std::async(&producer, std::ref(queue), 1000);
    futures.push_back(std::move(fut));

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "waiting..." << std::endl;

    std::for_each(futures.begin(), futures.end(), [](std::future<void>& fut) { fut.wait(); });

    return 0;
}