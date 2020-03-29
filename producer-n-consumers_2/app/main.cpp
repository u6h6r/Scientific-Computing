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
#include <mutex>
#include <condition_variable> 
// https://stackoverflow.com/questions/9015748/whats-the-difference-between-notify-all-and-notify-one-of-stdcondition-va

std::atomic<int> g_sorted;
const int g_array_size = 100000;

template< typename T > 
class FixedQueue: public std::queue< T > 
{ 
private:
    inline bool is_full() 
    {
        return !( current < max_size );
    }   

    const int max_size;
    int current = 0;

public:
    FixedQueue(int sajz) : max_size(sajz){} 
    void getMaxSize() 
    { 
        std::cout << "Max size is: " << max_size << "\n";
    }

    void push( T data )
    {   
        if (is_full()) 
        {
            std::cout << "Queue full" << std::endl;
        }   
        else 
        {
            std::queue< T >::push( data );
            ++current;
        }
    }

    void pop() 
    {   
        if (!std::queue< T >::empty()) 
        {
            std::queue< T >::pop();
            --current;
        }   
    }
};

class Producer
{
    public:
        // https://stackoverflow.com/questions/19661347/passing-template-class-as-parameter
        template<class T>
        Producer(FixedQueue<T> & fixque, int arrays_num)
        {
            produce(fixque, arrays_num);
        }

        template<class T>
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

class Consumer
{
    public:
        template<class T> 
        Consumer(FixedQueue< T > & fixque)
        {
            std::cout << "Queue size(class Consumer): " <<fixque.size() << "\n";
            while(!fixque.empty())
            {
                consume(fixque);
            }
            std::cout << "Queue size(class Consumer): " <<fixque.size() << "\n";
            std::cout << "Consumer has sorted " << current << " num of arrays\n";
        }

        template<class T>
        void consume(FixedQueue< T > & fixque)
        {
            std::cout << "Now sorting array num: CONSUMIG " << current << "\n";
            std::cout << "Queue size(class Consumer) before pop: " <<fixque.size() << "\n";
            std::array<int, g_array_size> arr(fixque.front());
            fixque.pop();
            int sum = check_sum(arr);
            std::cout << "Sum of elements of an array is: " << sum << "\n";
            std::cout << "Queue size(class Consumer) after pop: " <<fixque.size() << "\n";
            std::sort(arr.begin(), arr.end());
            ++current;
            ++g_sorted;
        }

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

    // std::array<int, g_array_size> ar1;
    // std::array<int, g_array_size> ar2;
    // std::array<int, g_array_size> ar3;
    // std::array<int, g_array_size> ar4;

    fque.getMaxSize();
    // fque.push(ar1);
    // fque.push(ar2);
    std::cout << fque.size() << "\n";
    Producer prod(fque, 4);
    Consumer cons(fque);
    // std::cout << "Consumer has sorted (atomic) " << g_sorted+1 << " num of arrays\n";
    // fque.push(ar4);
    // fque.pop();
    // fque.pop();
    // std::cout <<  fque.size() << "\n";
    // fque.push(ar3);
    // std::cout <<  fque.size() << "\n";

    return 0;
}