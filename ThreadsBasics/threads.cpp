#include <iostream>
#include <thread>
#include <string>
#include <mutex>
#include <chrono>
#include <vector>

typedef std::chrono::high_resolution_clock Clock;
static volatile int g_counter = 0;
static const int g_counter_stop = 10000000;
static std::mutex g_mutex;
static volatile std::atomic<int> g_atomic;


void t(std::string str)
{   
    std::cout << str << "\t - thread_id: "<< std::this_thread::get_id() << "\n";
}

void t_50(std::string str, int x)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    for (int i=0; i < 50; i++)
        std::cout << str << i+1 << "\t - thread_id_"<< x+1 << ": "<< std::this_thread::get_id() << "\n";
}

void increment()
{
    auto t_start = Clock::now();
    for(int i=0; i<g_counter_stop; ++i)
    {
        ++g_counter;
    }
    auto t_stop = Clock::now();
    typedef std::chrono::duration<float, std::milli> milli_seconds;
    std::cout << "Elapsed time:\t " << std::chrono::duration_cast<milli_seconds>(t_stop - t_start).count() << "ms\n";
}

void single_incerement()
{
    std::cout << "Single increment:" << "\n";
    std::cout << "g_counter before incrementation: " << g_counter << "\n";
    increment();
    std::cout << "g_counter after incrementation:\t " << g_counter << "\n";
}

void unsync_increment()
{
    for(int i=0; i<g_counter_stop; ++i)
        ++g_counter;
}

void mutex_increment()
{
    for(int i=0; i<g_counter_stop; ++i)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        ++g_counter;
    }
}

void atomic_increment()
{
    for(int i=0; i<g_counter_stop; ++i)
    {
        ++g_atomic;
    }
}

void multi_increment(void (*increment_func)(), std::string str)
{   
    if (str == "g_atomic")
        {
            std::cout << "Multi increment with atomic: \n";
            std::cout << "g_counter before incrementation: " << g_atomic << "\n";
        }
    if (str == "g_mutex")
        {
            g_counter = 0;
            std::cout << "Multi increment with mutex: \n";
            std::cout << "g_counter before incrementation: " << g_counter << "\n";
        }
    if (str == "g_unsync")
        {
            g_counter = 0;
            std::cout << "Multi increment unsynchronized: \n";
            std::cout << "g_counter before incrementation: " << g_counter << "\n";
        }
    int t_num = 10;
    std::vector<std::thread> t_vector;
    t_vector.reserve(t_num);
    auto t_start = Clock::now();
    for (int i = 0; i < t_num; i++)
        t_vector.emplace_back(std::thread(increment_func));
    for (auto& thread : t_vector)
        thread.join();
    auto t_stop = Clock::now();
    if (str == "g_atomic")
        {
            typedef std::chrono::duration<float, std::milli> milli_seconds;
            std::cout << "Elapsed time:\t\t " << std::chrono::duration_cast<milli_seconds>(t_stop - t_start).count() << "ms\n";
            std::cout << "g_counter after incrementation:\t " << g_atomic << "\n";
        }
    if (str == "g_mutex")
        {
            typedef std::chrono::duration<float, std::milli> milli_seconds;
            std::cout << "Elapsed time:\t\t " << std::chrono::duration_cast<milli_seconds>(t_stop - t_start).count() << "ms\n";
            std::cout << "g_counter after incrementation:\t " << g_counter << "\n";
        }
    if (str == "g_unsync")
        {
            typedef std::chrono::duration<float, std::milli> milli_seconds;
            std::cout << "Elapsed time:\t\t " << std::chrono::duration_cast<milli_seconds>(t_stop - t_start).count() << "ms\n";
            std::cout << "g_counter after incrementation:\t " << g_counter << "\n";
        }
}