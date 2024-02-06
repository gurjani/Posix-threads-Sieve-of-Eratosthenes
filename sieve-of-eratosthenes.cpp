#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <mutex>

std::mutex mtx; // Mutex for synchronization if needed

// Sequential Sieve of Eratosthenes for primes up to sqrt(Max)
void calculateSeeds(std::vector<bool> &seedPrimes, long sqrtMax) {
    seedPrimes.assign(sqrtMax + 1, true); // Initialize all numbers as potential primes

    for (long p = 2; p * p <= sqrtMax; ++p) {
        if (seedPrimes[p]) {
            for (long i = p * p; i <= sqrtMax; i += p) {
                seedPrimes[i] = false; // Mark multiples of primes as non-prime
            }
        }
    }
}

int main(int argc, char const *argv[]) {
    if (argc < 3) {
        std::cerr << "Please specify n and Max: " << std::endl;
        std::exit(EXIT_FAILURE);
    }

    int num_of_threads = std::stoi(argv[1]);
    long max = std::stoi(argv[2]);

    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point stop_time;

    start_time = std::chrono::high_resolution_clock::now();

    std::vector<bool> seedPrimes;
    long sqrtMax = static_cast<long>(std::sqrt(max));

    // Calculate seeds (primes up to sqrt(Max)) sequentially
    calculateSeeds(seedPrimes, sqrtMax);

    stop_time = std::chrono::high_resolution_clock::now();
    std::cout << "Precalculation: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count()
              << " ms"
              << std::endl;

    start_time = std::chrono::high_resolution_clock::now();

    // Calculate the remaining range from sqrt(Max) + 1 to Max
    long remainingRange = max - sqrtMax;
    int num_cores = static_cast<int>(std::min(static_cast<unsigned int>(num_of_threads), std::thread::hardware_concurrency()));
    long chunkSize = (remainingRange + num_cores - 1) / num_cores;

    std::vector<std::thread> threads(num_cores);

    for (int i = 0; i < num_cores; ++i) {
        long start = sqrtMax + 1 + i * chunkSize;
        long end = std::min(sqrtMax + 1 + (i + 1) * chunkSize, max + 1);

        threads[i] = std::thread([&seedPrimes, start, end]() {
            // Local marking of non-prime numbers using seedPrimes
            for (long p = 2; p * p <= end; ++p) {
                if (seedPrimes[p]) {
                    long start_multiple = std::max(p * p, (start + p - 1) / p * p); // Ensure multiples within the chunk
                    for (long i = start_multiple; i < end; i += p) {
                        mtx.lock(); // Lock for synchronization if needed
                        // Mark non-prime number in the chunk
                        // Example: nonPrimes[i - start] = false;
                        mtx.unlock();
                    }
                }
            }
        });
    }

    // Wait for all threads to finish
    for (auto &thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    stop_time = std::chrono::high_resolution_clock::now();
    std::cout << "Creating and execution of threads: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count()
              << " ms"
              << std::endl;

    return 0;
}