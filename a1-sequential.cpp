#include <iostream>
#include <thread>
#include <random>
#include <chrono>
#include <sstream>
#include <string>
#include <fstream>
#include <iomanip>
#include <queue>

#include "a1-helpers.hpp"

int main(int argc, char **argv)
{
    // process input arguments
    int num_threads = 1;
    bool no_exec_times = false, only_exec_times = false;; // reporting of time measurements
    std::string filename = "input.txt";
    parse_args(argc, argv, num_threads, filename, no_exec_times, only_exec_times);

    // start: actual program
    unsigned int primes = 0, nonprimes = 0;
    double mean = 0.0;

    std::vector<unsigned int> number_counts(10, 0);

    // measure time from this moment
    auto t1 = std::chrono::high_resolution_clock::now();

    std::ifstream ifs(filename);

    unsigned int count = 0;
    double sum = 0.0;

    // read the file (we assume there are only numbers in the file)
    while (!ifs.eof())
    {
        int num; 
        ifs >> num; 

        bool is_prime = kernel(num);
        
        if (is_prime) {
            primes++;
        } else {
            nonprimes++;
        }

        number_counts[num % 10]++;

        count++;
        sum += num;
    }

    mean = sum / count;

    ifs.close();

    // measure time until this moment
    auto t2 = std::chrono::high_resolution_clock::now();

    // priting the results
    if ( !only_exec_times ) {
        std::cout << "Primes: " << primes << std::endl;
        std::cout << "Nonprimes: " << nonprimes << std::endl;
        std::cout << "Mean: " << std::fixed << std::setprecision(2) << mean << std::endl;
        
        for ( int j = 0; j < 10; ++j) {
            std::cout << j+1 << ": " << number_counts[j] << std::endl;
        }

        if ( !no_exec_times ) {
            std::cout << "Elapsed time (" << num_threads << " threads): " << std::chrono::duration<double>(t2 - t1).count() << std::endl;
        }
    } else {
        std::cout << num_threads << "\t" << std::chrono::duration<double>(t2 - t1).count() << std::endl;
    }

    return 0;
}
