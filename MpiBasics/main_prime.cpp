#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

int is_prime(int nr) 
{
    if (nr < 2)
        return 0;
    for (int i = 2; i <= (int)sqrt(nr); i++)
        if ((nr % i) == 0)
                return 0;
    return 1;
}

void prime_counter(int num)
{

    int start, end, rank, size, counter, summary;
    double start_time,end_time;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    start_time = MPI_Wtime();

    start = (rank*num)/size;
    end = ((rank*num)/size)+(num/size);
    counter = 0;

    if (rank == 0)
    {
        for (int i = start; i <= end; i++) 
        {
            if (is_prime(i)) 
            {
                counter++;
            }
        }
        MPI_Reduce(&counter, &summary, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        end_time=MPI_Wtime();
        std::cout << "Prime numbers in number " << num << " are: " << summary << "\n";
        std::cout << "Occured time for: " << size << " processes was: " << end_time-start_time << "s\n";
    }
    else
    {
        for (int i = start; i <= end; i++) 
        {
            if (is_prime(i)) 
            {
                counter++;
            }
        }
        MPI_Reduce(&counter, &summary, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    }
    
}

int main(int argc, char* argv[])
{
    int number = atoi(argv[1]);
    MPI_Init(&argc, &argv);
    prime_counter(number);
    MPI_Finalize();
}