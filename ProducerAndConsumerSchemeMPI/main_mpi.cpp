#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <queue>
#include <typeinfo>
#include <array>
#include <random>
#include <iterator>
#include <chrono>

// mpicxx -std=c++11 -o main_mpi ./main_mpi.cpp

typedef std::chrono::high_resolution_clock Clock;
const int ARRSIZE = {100000};
static int arrays_to_produce = {1000};

template< typename T > 
class FixedQueue: public std::queue< T > 
{
private:
    const int max_size;
    std::queue< T > fix_queue;
    int cons_ready;
    int rank;
    int job_done = 0;

public:
    FixedQueue(int sajz) : max_size(sajz){}

    ~FixedQueue()
    {
        std::cout << "QUEUE OBJECT DELETED\n";
    }

    bool try_push(T& data)
    {
        if(fix_queue.size() == max_size)
        {
            return false;
        }

        else
        {
            MPI_Status status;
            MPI_Recv(&cons_ready, 1, MPI_INT, MPI_ANY_SOURCE,
                     MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Send(&job_done, 1, MPI_C_BOOL, status.MPI_SOURCE, 
                     1, MPI_COMM_WORLD);
            fix_queue.push(data);
            MPI_Send(&fix_queue.front(), ARRSIZE, MPI_INT, 
                     status.MPI_SOURCE, 0, MPI_COMM_WORLD);
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
    int arrays_num, produced_arrays, ready, done_cons, nranks;
    bool job_done = false;
public:
    Producer(FixedQueue< T > & _fixque, int _arrays_num) : fixque(_fixque), arrays_num(_arrays_num) 
    {
        MPI_Comm_size(MPI_COMM_WORLD, &nranks);
    }
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
                ++produced_arrays;
            }
        }

        while(true)
        {
            MPI_Status status;
            job_done = true;
            MPI_Recv(&ready, 1, MPI_INT, MPI_ANY_SOURCE,
                     MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Send(&job_done, 1, MPI_C_BOOL, status.MPI_SOURCE, 
                     1, MPI_COMM_WORLD);
            ++done_cons;
            if (done_cons == nranks - 1)
            {
                break;
            }
        }
    }

    template < typename iterator >
    void fill_array( iterator start, iterator end)
    {
        std::generate(start, end, [&] () {return rand() % 100;});
    }
};

class Consumer
{   
private:
    int sum = 0;
    int sorted_arrays = 0;
    int rank;
    bool job_done = false;
    bool ready = false;
    std::array<int, ARRSIZE> ar;
    // FixedQueue< T > & fixque;

public:
    Consumer()
    {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    }

    ~Consumer()
    {
        std::cout << "CONSUMER OBJECT DELETED\n";
    }

    void run()
    {
        while (true)
        {
        MPI_Send(&ready, 1, MPI_C_BOOL, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(&job_done, 1, MPI_C_BOOL, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (job_done == true)
        {
            break;
        }
        MPI_Recv(&ar, ARRSIZE, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        consume(ar);
        sorted_arrays++;
        }
        std::cout << "Consumer number: " << rank << " has consumed: " << sorted_arrays << "\n";
    }

    void consume(std::array<int, ARRSIZE> data)
    {
        int sum = check_sum(data);
        std::cout << "CONSUMER consumed array checksum: " << sum << "\n";
        std::sort(data.begin(), data.end());
    }

    int check_sum(std::array<int, ARRSIZE> & arr)
    {
        int sum = std::accumulate(arr.begin(), arr.end(), 0);
        return sum;
    }
};


int main(int argc, char** argv) 
{
    MPI_Init(&argc, &argv);

    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    auto t_start = Clock::now();

    if (rank == 0)
    {
        FixedQueue< std::array<int, ARRSIZE> > fque(100);
        Producer< std::array<int, ARRSIZE> > prod(std::ref(fque), arrays_to_produce);
        prod.run();
    }

    else
    {
        Consumer cons;
        cons.run();
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0)
    {
    auto t_stop = Clock::now();
    typedef std::chrono::duration<float, std::milli> milli_seconds;
    std::cout << "Elapsed time:\t " << std::chrono::duration_cast<milli_seconds>(t_stop - t_start).count() << "ms\n";
    }

    MPI_Finalize();
    return 0;
}
