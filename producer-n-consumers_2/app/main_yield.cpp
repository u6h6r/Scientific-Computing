#include <iostream>
#include <queue>
#include <typeinfo>
#include <functional>
#include <array>
#include <chrono>
#include <random>
#include <iterator>
#include <algorithm>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable> 
#include <chrono>

typedef std::chrono::high_resolution_clock Clock;
const int ARRSIZE {100000};
static int arrays_to_produce {1000};
std::atomic<int> sorted_num {0};
std::mutex mutex_consumer;


template< typename T > 
class FixedQueue: public std::queue< T > 
{
private:
    const int max_size;
    std::queue< T > fix_queue;
    std::mutex m;

public:
    FixedQueue(int sajz) : max_size(sajz){}
    ~FixedQueue()
    {
        std::cout << "QUEUE OBJECT DELETED\n";
    }

    bool try_push(T const& data)
    {
        std::lock_guard<std::mutex> lock(m);
        {
        if(fix_queue.size() == max_size)
        {
            return false;
        }
        else
        {
            fix_queue.push(data);
            return true;
        }
        }
    }

    bool try_pop(T & popped_value)
    {
        std::lock_guard<std::mutex> lock(m);
        {
        if(fix_queue.empty())
        {
            return false;
        }
        popped_value=fix_queue.front();
        fix_queue.pop();
        return true;
        }
    }   
};


template<class T>
class Producer
{
private:
    FixedQueue< T > & fixque;
    int arrays_num;
    int produced_arrays = 0;

public:
    Producer(FixedQueue< T > & _fixque, int _arrays_num) : fixque(_fixque), arrays_num(_arrays_num) {};
    ~Producer()
    {
        std::cout << "PRODUCER OBJECT DELETED\n";
    }

    void run()
    {
        produce(fixque, arrays_num);
    }

    void produce(FixedQueue<T> & fixque, int arrays_num)
    {
        while(!(produced_arrays == arrays_to_produce))
        {
            std::array<int, ARRSIZE> ar;
            fill_array(ar.begin(), ar.end());
            if (fixque.try_push(ar))
            {
                std::cout << "PRODUCER produced array num: " << produced_arrays+1 << "\n";
                ++produced_arrays;
            }
            else
                {
                    std::this_thread::yield();
                }
        }
    }

    template < typename iterator >
    void fill_array( iterator start, iterator end)
    {
        static std::random_device rd;
        static std::mt19937 generator(rd());
        static std::uniform_int_distribution<int> rand(0,10000);
        std::generate(start, end, [&] () {return rand(generator);});
    }
};


template<class T>
class Consumer
{   
private:
    int sum = 0;
    int sorted_arrays = 0;
    FixedQueue< T > & fixque;
public:
    Consumer(FixedQueue< T > & _fixque) : fixque(_fixque) {};
    ~Consumer()
    {
        std::cout << "CONSUMER OBJECT DELETED\n";
    }

    void run()
    {
        while(!(arrays_to_produce == sorted_num))
        {
            std::array<int, ARRSIZE> arr;
            if(fixque.try_pop(arr))
            {
                consume(arr);
            }
            else
                std::this_thread::yield();
        }
        std::lock_guard<std::mutex> lock(mutex_consumer);
        std::cout << "Thread id " << std::this_thread::get_id() << " Consumer has sorted: " << sorted_arrays << "\n";
    }

    void consume(T & data)
    {
        int sum = check_sum(data);
        std::cout << "CONSUMER consumed array checksum: " << sum << "\n";
        std::sort(data.begin(), data.end());
        ++sorted_arrays;
        ++sorted_num;
    }

    int check_sum(std::array<int, ARRSIZE> & arr)
    {
        int sum = std::accumulate(arr.begin(), arr.end(), 0);
        return sum;
    }
};


class thread_guard
{
    std::thread& t;
public:
    explicit thread_guard(std::thread& t_): t(t_){}
    ~thread_guard()
    {
        if(t.joinable())
        {
            t.join();
        }
    }
    thread_guard(thread_guard const&)=delete;
    thread_guard& operator=(thread_guard const&)=delete;
};

int main()
{
    auto t_start = Clock::now();
    int consum_th_num = 10;

    FixedQueue< std::array<int, ARRSIZE> > fque(1000);

    Producer< std::array<int, ARRSIZE> > prod(std::ref(fque), arrays_to_produce);
    std::thread producer_t(&Producer< std::array<int, ARRSIZE> >::run, &prod);
    thread_guard prod_g(producer_t);

    std::vector<std::thread> t_vector;
    t_vector.reserve(consum_th_num);
    for (int i = 0; i < consum_th_num; i++)
    {
        Consumer< std::array<int, ARRSIZE> > cons(std::ref(fque));
        t_vector.emplace_back(std::thread(&Consumer< std::array<int, ARRSIZE> >::run, cons));
    }
    for (auto& thread : t_vector)
        thread_guard cons_g(thread);

    auto t_stop = Clock::now();
    typedef std::chrono::duration<float, std::milli> milli_seconds;
    std::cout << "Elapsed time:\t " << std::chrono::duration_cast<milli_seconds>(t_stop - t_start).count() << "ms\n";
    
    return 0;
}