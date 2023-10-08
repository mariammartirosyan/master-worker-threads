#include <iostream>
#include <thread>
#include <random>
#include <chrono>
#include <complex>
#include <sstream>
#include <string>
#include <fstream>
#include <iomanip>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>

#include "a1-helpers.hpp"

bool done=false;
const size_t buf_size = 100;

template <typename T>
class SafeQ
{
private:
    std::queue<T> q;
    std::mutex m;
    std::condition_variable cond;

public:
    void push(T value)
    {
        std::lock_guard<std::mutex> lock(m);
        q.push(value);
    }

    void pop(T &value)
    {
        std::unique_lock<std::mutex> lock(m);
        cond.wait(lock, [this] { return !q.empty(); });
        if (!q.empty()) {
            value = q.front();
            q.pop();
        }
    }
    void pop(std::vector<T>& buf, size_t n)
    {
        std::unique_lock<std::mutex> lock(m);
        cond.wait(lock, [this] { return q.size()>=buf_size;; });

        for (size_t i = 0; i < n && !q.empty(); ++i) {
            buf.push_back(q.front());
            q.pop();
        }
    }

    size_t size()
    {
        std::lock_guard<std::mutex> lock(m);
        return q.size();
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lock(m);
        return q.empty();
    }

    void notify_all()
    {
        cond.notify_all();
    }
    void notify_one()
    {
        cond.notify_one();
    }
};

int producer(std::string filename, SafeQ<int> &q)
{
    int produced_count = 0;
    std::ifstream ifs(filename);

    while (!ifs.eof()) {
        int num;
        ifs >> num;
        //add elements to the queue
        q.push(num);
       //when the queue size is equal to the buffer size notify a waiting thread
        if(q.size()==buf_size){
            q.notify_one();
        }
        produced_count++;
    }
    ifs.close();
    //when all the elements are read from the file notify all the waiting threads
    done = true;
    q.notify_all();
    return produced_count;
}

void worker(SafeQ<int> &q, std::atomic<int> &primes, std::atomic<int> &nonprimes, std::atomic<double> &sum, std::atomic<int> &consumed_count, std::vector<std::atomic<int>>& number_counts) {
  //define local variables so that each thread has its own copy
    thread_local double sum_local =0.0;
    thread_local int consumed_count_local =0, primes_local = 0,nonprimes_local=0;
    thread_local std::vector<unsigned int> number_counts_local(10, 0);
    
    while (true) {
        if (done && q.empty()) {
            break;
        }
        std::vector<int> buf;
        //add elements to the buffer from the queue
        q.pop(buf,buf_size);
        //do the calculations on local variables
        for (int num : buf) {
            consumed_count_local++;
            if (kernel(num) == 1) {
                primes_local++;
            } else {
                nonprimes_local++;
            }
            number_counts_local[num % 10]++;
            sum_local += num;
        }
    }
    //add the local variables to the atomic ones
    consumed_count.fetch_add(consumed_count_local);
    primes.fetch_add(primes_local);
    nonprimes.fetch_add(nonprimes_local);
    sum.store(sum.load()+sum_local);
    for (int i = 0; i < 10; i++) {
        number_counts[i].fetch_add(number_counts_local[i]);
    }
}
int main(int argc, char **argv)
{
    int num_threads = 8; // you can change this default to std::thread::hardware_concurrency()
    bool no_exec_times = false, only_exec_times = false;; // reporting of time measurements
    std::string filename = "input.txt";
    parse_args(argc, argv, num_threads, filename, no_exec_times, only_exec_times);

    // The actuall code
    double mean = 0.0;
    std::atomic<double> sum(0.0);
    std::atomic<int> primes(0), nonprimes(0), consumed_count(0);
    std::vector<std::atomic<int>> number_counts(10);
    
    // Queue that needs to be made safe 
    // In the simple form it takes integers 
    SafeQ<int> q;
    
    // put you worker threads here
    std::vector<std::thread> workers;

    // time measurement
    auto t1 =  std::chrono::high_resolution_clock::now();
    
    // implement: call the producer function with futures/async 
    std::future<int> produced_count_future = std::async(std::launch::async, producer, filename, std::ref(q));

    // implement: spawn worker threads - transform to spawn num_threads threads and store in the "workers" vector
    for (int i=0;i<num_threads;++i) {
        workers.push_back(std::thread(worker, std::ref(q), std::ref(primes), std::ref(nonprimes), std::ref(sum),std::ref(consumed_count),std::ref(number_counts)));
    }

    for (auto &worker : workers) {
        worker.join();
    }
    int produced_count = produced_count_future.get();
    mean = sum/consumed_count;
    // end time measurement
    auto t2 =  std::chrono::high_resolution_clock::now();
    // do not remove
    if ( produced_count != consumed_count ) {
         std::cout << "[error]: produced_count (" << produced_count << ") != consumed_count (" << consumed_count << ")." <<  std::endl;
    }
    
    std::vector<int> number_counts_int(number_counts.size());

    for (size_t i = 0; i < number_counts_int.size(); ++i) {
        number_counts_int[i] = number_counts[i].load();
    }
    // priting the results
    print_output(num_threads, primes, nonprimes, mean, number_counts_int, t1, t2, only_exec_times, no_exec_times);
    return 0;
}