# Master And Worker Threads

## Problem Overview
The serial code processes the numbers from the `input.txt` file and computes the following statistics:
- Total count of prime and non-prime numbers
- Mean of the numbers
- Count of numbers ending with each digit (0-9)

Parallel versions with **atomic** operations and **mutexes** were implemented where:
- The master thread parses the data from the file and inserts it into a queue.
- Worker threads perform computations on the elements.

## Parallel Implementation

### Master and Worker Threads
- The master thread asynchronously runs the producer function, which pushes items to the queue and returns the count of items pushed.
- Worker threads are spawned in a loop with the specified number of threads.
- When the master thread has added a specified number of elements to the queue, it notifies a worker thread, which adds the elements to a buffer and processes them. 
- While the worker thread processes elements, the queue remains available for the master thread to add more items, enabling parallel execution.
- All threads are notified once the master thread completes the task of adding elements to the queue.

### Dynamic Work Distribution
- The work is distributed dynamically among the threads. Once a batch of `buf_size` elements is in the queue, a thread is notified and starts consuming elements in batches.
- The **dynamic distribution** approach outperforms the static one by accounting for each thread's workload and balancing it more effectively, whereas static distribution may lead to underutilization of resources due to pre-assigned work that doesn't consider individual thread workloads.

## Synchronization and SafeQ Class
- The `SafeQ` class uses a mutex and a condition variable to ensure safe access to the queue by multiple threads.
    - `std::lock_guard` is used to lock the queue during `push`, `size`, and `empty` operations.
    - `std::unique_lock` is used in the `pop` method. The method contains a condition variable that notifies threads to wait for more items. Once notified and the queue contains `buf_size` elements, the thread continues its execution.
- To ensure thread synchronization on shared variables in the worker method,  **atomic** operations and **mutexes** are used depending on the implementation.

## Performance Optimization

### Buffering
One performance bottleneck was resolved by introducing a buffer. This ensured that master and worker threads worked in parallel and prevented the queue from being locked while the worker executed tasks.

### Thread-Local Variables
Another optimization involved the use of `thread_local` variables. This approach eliminated the need to lock shared variables for every element, as each thread works on its local copy and only updates the shared variable once the thread's computations are complete.

## Performance Measurements

### Atomic Version
| Number of Threads | Execution Time (Sec) | Speedup |
|-------------------|--------------------|---------|
| 2                 | 5.04               | 2       |
| 4                 | 2.55               | 3.9     |
| 6                 | 1.81               | 5.6     |
| 8                 | 1.42               | 7.1     |
| 10                | 1.13               | 9       |
| 12                | 0.96               | 10.6    |
| 14                | 0.85               | 11.9    |
| 16                | 0.74               | 13.7    |
| 18                | 0.74               | 13.7    |
| 20                | 0.75               | 13.5    |
| 22                | 0.74               | 13.7    |
| 24                | 0.88               | 11.5    |
| 26                | 0.74               | 13.7    |
| 28                | 0.74               | 13.7    |
| 30                | 0.88               | 11.5    |
| 32                | 0.75               | 13.5    |

### Mutex Version
| Number of Threads | Execution Time (Sec) | Speedup |
|-------------------|--------------------|---------|
| 2                 | 5.05               | 2       |
| 4                 | 2.59               | 3.9     |
| 6                 | 1.83               | 5.5     |
| 8                 | 1.43               | 7.1     |
| 10                | 1.13               | 9       |
| 12                | 0.96               | 10.6    |
| 14                | 0.84               | 12.1    |
| 16                | 0.78               | 13      |
| 18                | 0.78               | 13      |
| 20                | 0.74               | 13.7    |
| 22                | 0.74               | 13.7    |
| 24                | 0.74               | 13.7    |
| 26                | 0.85               | 11.9    |
| 28                | 0.76               | 13.3    |
| 30                | 0.76               | 13.3    |
| 32                | 0.76               | 13.3    |

## Conclusion
Both atomic and mutex versions have similar results, achieving a maximum speedup of 13.7x compared to the sequential version. Performance improvements were mainly achieved by introducing a buffer and using thread-local variables to reduce the frequency of access to shared variables. The dynamic distribution of work further enhanced performance by balancing workloads across threads more effectively than static distribution.