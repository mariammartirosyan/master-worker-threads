
/**
 * Check if the number is prime.
 * Do not modify this function!
 *
 * @param[in] number
 * @returns integer
 *
 */
bool kernel(int number)
{
    // kernel is sensitive to input
    bool prime = true;
    for (int i = 2; i <= number / 2; ++i)
    {
        if (number % i == 0)
        {
            prime = false;
        }
    }
    return prime;
};

/**
 * Pretty printing
 *
 */
void print_output(int num_threads, int primes, int nonprimes, double mean, std::vector<int> number_counts, std::chrono::high_resolution_clock::time_point t1, std::chrono::high_resolution_clock::time_point t2, bool only_exec_times, bool no_exec_times)
{
    if (!only_exec_times)
    {
        std::cout << "Primes: " << primes << std::endl;
        std::cout << "Nonprimes: " << nonprimes << std::endl;
        std::cout << "Mean: " << std::fixed << std::setprecision(2) << mean << std::endl;

        for (int j = 0; j < 10; ++j)
        {
            std::cout << j + 1 << ": " << number_counts[j] << std::endl;
        }

        if (!no_exec_times)
        {
            std::cout << "Elapsed time (" << num_threads << " threads): " << std::chrono::duration<double>(t2 - t1).count() << std::endl;
        }
    }
    else
    {
        std::cout << num_threads << "\t" << std::chrono::duration<double>(t2 - t1).count() << std::endl;
    }
}

/**
 * Parsing arguments
 *
 */
void parse_args(int argc, char **argv, int &num_threads, std::string &filename, bool &no_exec_times, bool &only_exec_times)
{
    std::string usage("Usage: <input-filename> --num-worker-threads <integer> --no-exec-times --only-exec-times");

    for (int i = 0; i < argc; ++i) {
        if (std::string(argv[i]).compare("--num-threads") == 0)
        {
            num_threads = atoi(argv[++i]);
        }
        else if (std::string(argv[i]).compare("--no-exec-times") == 0)
        {
            no_exec_times = true;
        }
        else if (std::string(argv[i]).compare("--only-exec-times") == 0)
        {
            only_exec_times = true;
        }
        else if (std::string(argv[i]).compare("--help") == 0)
        {
            std::cout << usage << std::endl;
            exit(-1);
        } else {
            filename = i > 0 ? std::string(argv[i]) : filename;
            if (!std::ifstream(filename, std::ifstream::in).good())
            {
                std::cerr << "[error]: could not open input file '" << filename << "'!" << std::endl;
                exit(-1);
            }
        }
    }
};