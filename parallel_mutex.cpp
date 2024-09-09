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

#include "helpers.hpp"

bool done = false;
const size_t buf_size = 100;
std::mutex count_m;
std::mutex prime_m;
std::mutex nonprime_m;
std::mutex numcount_m;
std::mutex sum_m;

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
        cond.wait(lock, [this] { return q.size()>=buf_size; });

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
        q.push(num);
        if(q.size()==buf_size){
            q.notify_one();
        }
        produced_count++;
    }
    ifs.close();
    done = true;
    q.notify_all();
    return produced_count;
}

void worker(SafeQ<int> &q, int &primes, int &nonprimes, double &sum, int &consumed_count, std::vector<int>& number_counts) {
    
    // define local variables so that each thread has its own copy
    thread_local double sum_local=0.0;
    thread_local int consumed_count_local =0, primes_local = 0,nonprimes_local=0;
    thread_local std::vector<unsigned int> number_counts_local(10, 0);

    while (true) {
        if (done && q.empty()) {
            break;
        }

        std::vector<int> buf;

        // add elements to the buffer from the queue
        q.pop(buf,buf_size);

        // do the calculations on local variables
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
    // add the local variables to the shared ones while ensuring thread-safe access by mutexes
    {
        std::lock_guard<std::mutex> lock(count_m);
        consumed_count+=consumed_count_local;
    }
    {
        std::lock_guard<std::mutex> lock(prime_m);
        primes+=primes_local;
    }
    {
        std::lock_guard<std::mutex> lock(nonprime_m);
        nonprimes+=nonprimes_local;
    }
    {
        std::lock_guard<std::mutex> lock(sum_m);
        sum += sum_local;
    }
    {
        std::lock_guard<std::mutex> lock(numcount_m);
        for (int i = 0; i < 10; i++) {
            number_counts[i] += number_counts_local[i];
        }
    }
}

int main(int argc, char **argv)
{
    int num_threads = 8; 
    
    bool no_exec_times = false, only_exec_times = false;; // reporting of time measurements
    std::string filename = "input.txt";
    parse_args(argc, argv, num_threads, filename, no_exec_times, only_exec_times);

    int primes = 0, nonprimes = 0, count = 0;
    int consumed_count = 0;
    double mean = 0.0, sum = 0.0;
    // vector for storing numbers ending with different digits (0-9)
    std::vector<int> number_counts(10, 0);
    
    SafeQ<int> q;
    std::vector<std::thread> workers;

    // time measurement
    auto t1 =  std::chrono::high_resolution_clock::now();
    
    std::future<int> produced_count_future = std::async(std::launch::async, producer, filename, std::ref(q));
    // spawn worker threads
    for (int i=0;i<num_threads;++i) {
        workers.push_back(std::thread(worker, std::ref(q), std::ref(primes), std::ref(nonprimes), std::ref(sum),std::ref(consumed_count),std::ref(number_counts)));
    }
    int produced_count = produced_count_future.get();
    for (auto &worker : workers) {
    worker.join();
    }
    mean = sum/consumed_count;
    // end time measurement
    auto t2 =  std::chrono::high_resolution_clock::now();

    if ( produced_count != consumed_count ) {
         std::cout << "[error]: produced_count (" << produced_count << ") != consumed_count (" << consumed_count << ")." <<  std::endl;
    }
    // priting the results
    print_output(num_threads, primes, nonprimes, mean, number_counts, t1, t2, only_exec_times, no_exec_times);
    return 0;
}