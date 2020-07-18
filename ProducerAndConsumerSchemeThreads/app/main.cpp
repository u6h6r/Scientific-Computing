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
// https://stackoverflow.com/questions/9015748/whats-the-difference-between-notify-all-and-notify-one-of-stdcondition-va
#include <chrono>

typedef std::chrono::high_resolution_clock Clock;
const int g_array_size = 100000;
// std::atomic<bool> done {false};
static int arrays_to_produce = 1000;
// bool queue_empty {false};
// bool job_done {false};
std::atomic<int> sorted_num {0};

std::condition_variable cv_need_item;
std::condition_variable cv_has_item;
std::mutex mutex_consumer;
// std::mutex mutex_finish;

template< typename T > 
class FixedQueue: public std::queue< T > 
{
private:
    // std::condition_variable cv_need_item;
    // std::condition_variable cv_has_item;
    // std::condition_variable cv;
    // std::mutex m_empty;
    // std::mutex m_full;

    mutable std::mutex m;
    const int max_size;
    std::queue< T > fix_queue;
    // std::atomic<int> current {0};
    // bool queue_full {false};

public:
    FixedQueue(int sajz) : max_size(sajz){}
    // void getMaxSize() 
    // { 
    //     std::cout << "Max size is: " << max_size << "\n";
    // }
    bool empty() const
    {
        // std::unique_lock<std::mutex> lock(m);
        std::unique_lock<std::mutex> lock(m);
        std::cout << "PUSTA\n";
        // queue_empty = std::queue< T >::empty();
        return fix_queue.empty();
    }

    void wait_and_push(T const& data)
    {
        std::unique_lock<std::mutex> lock(m);
        while(fix_queue.size() == max_size)
        {
            // std::this_thread::yield();
            // std::cout << "Queue is full... wiating\n";
            cv_need_item.wait(lock);
        }
        // cv_need_item.wait(lock, [&]{(fix_queue.size() == max_size);});
        fix_queue.push(data);
        lock.unlock();
        cv_has_item.notify_one();
    }

    bool try_pop(T & popped_value)
    {
        std::lock_guard<std::mutex> lock(m);
        if(fix_queue.empty())
        {
            return false;
        }
        
        popped_value=fix_queue.front();
        fix_queue.pop();
        cv_need_item.notify_one();
        return true;
    }

    void wait_and_pop(T & popped_value)
    {
        if (!(arrays_to_produce == sorted_num))
        {
        std::unique_lock<std::mutex> lock(m);
        while(fix_queue.empty())
        {
            // if (arrays_to_produce == sorted_num)
            // {
            //     cv_has_item.notify_all();
            //     return;
            // }
            // std::this_thread::yield();
            cv_has_item.wait(lock);
        }
        // cv_has_item.wait(lock, [&]{fix_queue.empty();});
        popped_value=fix_queue.front();
        fix_queue.pop();
        lock.unlock();
        cv_need_item.notify_one();
        }

    }
    
};

template<class T>
class Producer
{
private:
    FixedQueue< T > & fixque;
    int arrays_num;

public:
    // https://stackoverflow.com/questions/19661347/passing-template-class-as-parameter
    Producer(FixedQueue< T > & _fixque, int _arrays_num) : fixque(_fixque), arrays_num(_arrays_num) {};
    void run()
    {
        produce(fixque, arrays_num);
    }
    void produce(FixedQueue<T> & fixque, int arrays_num)
    {
        for(int i=0; i<arrays_num; i++)
        {
            std::cout << "Producer is producing array num: PRODUCER " << i+1 << "\n";
            std::array<int, g_array_size> ar;
            fill_array(ar.begin(), ar.end());
            // int sum = std::accumulate(ar.begin(), ar.end(), 0);

            fixque.wait_and_push(ar);

            // std::cout << "Queue size(class Producer): " <<fixque.size() << "\n";
        }
    }
    // https://stackoverflow.com/questions/972152/how-to-create-a-template-function-within-a-class-c
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

    void run()
    {
        while(!(arrays_to_produce == sorted_num))
        {
            std::lock_guard<std::mutex> lock(mutex_consumer);
            std::array<int, g_array_size> arr;
            if(!fixque.try_pop(arr))
            {   
                // std::this_thread::yield();
                // std::cout<< "Try nie dal rady PUSH \n";
                fixque.wait_and_pop(arr);
            }
            // consume(arr);
            if(sorted_num < arrays_to_produce)
                consume(arr);
        }
        std::lock_guard<std::mutex> lock(mutex_consumer);
        if (arrays_to_produce == sorted_num)
        {
            std::cout << "Thread id " << std::this_thread::get_id() << " Consumer has sorted: " << sorted_arrays << "\n";
            cv_has_item.notify_all();
        }

    }
    void consume(T & data)
    {
        int sum = check_sum(data);
        std::cout << "(CONSUME)Sum of elements of an array is: " << sum << "\n";
        std::sort(data.begin(), data.end());
        ++sorted_arrays;
        // std::lock_guard<std::mutex> lock(mutex_final);
        ++sorted_num;
    }
    int check_sum(std::array<int, g_array_size> & arr)
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
    int t_num = 10;
    FixedQueue< std::array<int, g_array_size> > fque(100);

    Producer< std::array<int, g_array_size> > prod(std::ref(fque), arrays_to_produce);
    std::thread producer_t(&Producer< std::array<int, g_array_size> >::run, &prod);
    thread_guard prod_g(producer_t);

    std::vector<std::thread> t_vector;
    t_vector.reserve(t_num);
    for (int i = 0; i < t_num; i++)
    {
        Consumer< std::array<int, g_array_size> > cons(std::ref(fque));
        t_vector.emplace_back(std::thread(&Consumer< std::array<int, g_array_size> >::run, cons));
    }
    for (auto& thread : t_vector)
        thread_guard cons_g(thread);
    auto t_stop = Clock::now();
    typedef std::chrono::duration<float, std::milli> milli_seconds;
    std::cout << "Elapsed time:\t " << std::chrono::duration_cast<milli_seconds>(t_stop - t_start).count() << "ms\n";

    return 0;
}