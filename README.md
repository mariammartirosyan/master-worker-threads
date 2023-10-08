# MasterWorkerThreads
The serial code analyzes the numbers of "input.txt" file and displays statistics, such as the total number of prime and non-prime numbers in the set, the mean, and counts the numbers that end with different digits (0-9).

Implemented a parallel version where master thread parses data from the file and puts them in a queue, while the worker threads do the computations.

I made two versions: one with the worker() function using only atomics and one where the worker() is using mutexes.
