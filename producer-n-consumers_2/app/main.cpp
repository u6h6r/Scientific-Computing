#include <iostream>
#include <queue>
#include <typeinfo>
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

std::atomic<int> g_sorted;
const int g_array_size = 100000;

template< typename T > 
class FixedQueue: public std::queue< T > 
{ 
private:
    std::condition_variable cv;
    std::mutex m;
    inline bool is_full() 
    {
        return !( a_current < max_size );
    }   

    const int max_size;
    // int current = 0;
    std::atomic<int> a_current {0};

public:
    FixedQueue(int sajz) : max_size(sajz){} 
    void getMaxSize() 
    { 
        std::cout << "Max size is: " << max_size << "\n";
    }

    // void push( T data )
    // {   
    //     if (is_full()) 
    //     {
    //         std::cout << "Queue full" << std::endl;
    //     }   
    //     else 
    //     {
    //         std::queue< T >::push( data );
    //         ++current;
    //     }
    // }
    // void pop() 
    // {   
    //     if (!std::queue< T >::empty()) 
    //     {
    //         std::queue< T >::pop();
    //         --current;
    //     }   
    // }

    void push(T const& data)
    {
 
        std::unique_lock<std::mutex> lock(m);
        bool const was_empty=std::queue< T >::empty();
        std::queue< T >::push(data);
        ++a_current;
        std::cout << "Queue is empty: " << was_empty << "\n";
        std::cout << "Queue is full: " << is_full() << "\n";
        lock.unlock(); // unlock the mutex

        if(was_empty)
        {
            cv.notify_one();
        }

    }

    void wait_and_pop(T& popped_data)
    {
        std::unique_lock<std::mutex> lock(m);
        while(std::queue< T >::empty())
        {
            cv.wait(lock);
        }
        std::cout << "DATA POPPED FORM QUEUE\n";
        popped_data=std::queue< T >::front();
        std::queue< T >::pop();
        --a_current;
    }

};

template<class T>
class Producer
{
    FixedQueue< T > & fixque;
    int arrays_num;
    public:
        // https://stackoverflow.com/questions/19661347/passing-template-class-as-parameter
        // template<class T>
        Producer(FixedQueue< T > & _fixque, int _arrays_num) : fixque(_fixque), arrays_num(_arrays_num) {};
        // {
        //     produce(fixque, arrays_num);
        // }

        // template<class T>
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
                int sum = std::accumulate(ar.begin(), ar.end(), 0);
                // std::cout << "SUM from checksum PRODUCER: " << sum << "\n";
                // if(i == 0)
                // {
                // for (auto num: ar) 
                // {
                //     std::cout << num << ", ";
                // }
                // std::cout << "\n";
                // }
                // v_of_a.emplace_back(ar);
                fixque.push(ar);
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
    FixedQueue< T > & fixque;
    public:
        // template<class T> 
        Consumer(FixedQueue< T > & _fixque) : fixque(_fixque) {};
        // {

        //     std::array<int, g_array_size> arr;
        //     fixque.wait_and_pop(arr);
        //     consume(arr);
        //     // int sum = check_sum(arr);
        //     // std::cout << "Sum of elements of an array is: " << sum << "\n";
        //     // std::cout << "Queue size(class Consumer): " <<fixque.size() << "\n";
        //     // while(!fixque.empty())
        //     // {
        //     //     consume(fixque);
        //     // }
        //     // std::cout << "Queue size(class Consumer): " <<fixque.size() << "\n";
        //     // std::cout << "Consumer has sorted " << current << " num of arrays\n";
        // }
        void run()
        {
            std::array<int, g_array_size> arr;
            fixque.wait_and_pop(arr);
            // std::cout << "Consumer_id: " << str << " is consuming\n";
            consume(arr);
        }
        // template<class T> 
        void consume(T & data)
        {
            int sum = check_sum(data);
            std::cout << "(CONSUME)Sum of elements of an array is: " << sum << "\n";
            std::sort(data.begin(), data.end());
            ++g_sorted;
        }

        // template<class T>
        // void consume(FixedQueue< T > & fixque)
        // {
        //     std::cout << "Now sorting array num: CONSUMIG " << current << "\n";
        //     std::cout << "Queue size(class Consumer) before pop: " <<fixque.size() << "\n";
        //     std::array<int, g_array_size> arr(fixque.front());
        //     fixque.pop();
        //     int sum = check_sum(arr);
        //     std::cout << "Sum of elements of an array is: " << sum << "\n";
        //     std::cout << "Queue size(class Consumer) after pop: " <<fixque.size() << "\n";
        //     std::sort(arr.begin(), arr.end());
        //     ++current;
        //     ++g_sorted;
        // }

        int check_sum(std::array<int, g_array_size> & arr)
        {
            int sum = std::accumulate(arr.begin(), arr.end(), 0);
            // std::cout << "SUM from checksum: " << sum << "\n";
            return sum;
        }

    private:
        int sum = 0;
        int current = 0;
};

int main()
{
    FixedQueue< std::array<int, g_array_size> > fque(3);
    // Producer prod(fque);
    int t_num = 10;
    // std::array<int, g_array_size> ar1;
    // std::array<int, g_array_size> ar2;
    // std::array<int, g_array_size> ar3;
    // std::array<int, g_array_size> ar4;

    fque.getMaxSize();
    // fque.push(ar1);
    // fque.push(ar2);
    std::cout << fque.size() << "\n";
    Producer< std::array<int, g_array_size> > prod(fque, 10);
    std::thread producer_t(&Producer< std::array<int, g_array_size> >::run, prod);
    // producer_t.join();
    // Consumer< std::array<int, g_array_size> > cons(fque);
    // cons.run();
    // Consumer< std::array<int, g_array_size> > cons_2(fque);
    // cons_2.run();

    // Consumer< std::array<int, g_array_size> > cons(fque);
    // std::thread t1(&Consumer< std::array<int, g_array_size> >::run, cons);
    // t1.join();

    std::vector<std::thread> t_vector;
    t_vector.reserve(t_num);
    for (int i = 0; i < t_num; i++)
    {
        Consumer< std::array<int, g_array_size> > cons(fque);
        t_vector.emplace_back(std::thread(&Consumer< std::array<int, g_array_size> >::run, cons));
    }
    for (auto& thread : t_vector)
        thread.join();
    producer_t.join();
    // Consumer cons(fque, "1");
    // Consumer cons_2(fque, "2");
    // Consumer cons_3(fque, "3");
    std::cout << "Consumer has sorted (atomic) " << g_sorted << " num of arrays\n";
    // fque.push(ar4);
    // fque.pop();
    // fque.pop();
    // std::cout <<  fque.size() << "\n";
    // fque.push(ar3);
    // std::cout <<  fque.size() << "\n";

    return 0;
}