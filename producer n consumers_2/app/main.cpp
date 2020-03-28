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

std::atomic<int> g_sorted;

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

    void push( T value )
    {   
        if (is_full()) 
        {
            std::cout << "Queue full" << std::endl;
        }   
        else 
        {
            std::queue< T >::push( value );
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
        Producer(FixedQueue< std::array<int, 100000> > & fixque, int arrays_num)
        {
            // std::cout << "Num of arrays to produce is: " << arrays_num << "\n";
            // std::array<int, 100000> ar5;
            // std::array<int, 100000> ar6;
            // fixque.push(ar5);
            // std::cout << "Size z class Producer: " <<fixque.size() << "\n";
            // fixque.push(ar6);
            // std::cout << "Queue is not empty: " << !fixque.empty() << "\n";
            // fill_array();
            // std::vector<std::array<int, 100000>> v_of_a;
            // v_of_a.reserve(arrays_num);
            for(int i=0; i<arrays_num; i++)
            {
                std::cout << "Producer is producing array num: " << i+1 << "\n";
                std::array<int, 100000> ar;
                fill_array(ar.begin(), ar.end());
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
                std::cout << "Queue size(class Producer): " <<fixque.size() << "\n";
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
        Consumer(FixedQueue< std::array<int, 100000> > & fixque)
        {
            std::cout << "Queue size(class Consumer): " <<fixque.size() << "\n";
            while(!fixque.empty())
            {
                // if(current == 2)
                // {
                //     std::array<int, 100000> & arr = fixque.front();
                //     fixque.pop();
                //     std::sort(arr.begin(), arr.end());
                //     for (auto num: arr) 
                //     {
                //         std::cout << num << ", ";
                //     }
                //     std::cout << "\n";
                //     ++current;
                // }
                std::cout << "Now sorting array num: " << current << "\n";
                std::cout << "Queue size(class Consumer) before pop: " <<fixque.size() << "\n";
                std::array<int, 100000> & arr = fixque.front();
                fixque.pop();
                sum = check_sum(arr);
                std::cout << "Sum of elements of an array is: " << sum << "\n";
                std::cout << "Queue size(class Consumer) after pop: " <<fixque.size() << "\n";
                std::sort(arr.begin(), arr.end());
                sum = 0;
                ++current;
                ++g_sorted;
            }
            std::cout << "Queue size(class Consumer): " <<fixque.size() << "\n";
            std::cout << "Consumer has sorted " << current+1 << " num of arrays\n";
        }
        int check_sum(std::array<int, 100000> & arr)
        {
            int sum = std::accumulate(arr.begin(), arr.end(), 0);
            return sum;
        }
    private:
        int sum = 0;
        int current = 0;
};

int main()
{
    FixedQueue< std::array<int, 100000> > fque(3);
    // Producer prod(fque);

    std::array<int, 100000> ar1;
    std::array<int, 100000> ar2;
    std::array<int, 100000> ar3;
    std::array<int, 100000> ar4;

    fque.getMaxSize();
    fque.push(ar1);
    // fque.push(ar2);
    std::cout << fque.size() << "\n";
    Producer prod(fque, 4);
    Consumer cons(fque);
    std::cout << "Consumer has sorted (atomic) " << g_sorted+1 << " num of arrays\n";
    // fque.push(ar4);
    // fque.pop();
    // fque.pop();
    // std::cout <<  fque.size() << "\n";
    // fque.push(ar3);
    // std::cout <<  fque.size() << "\n";

    return 0;
}