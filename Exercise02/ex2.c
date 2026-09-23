#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    int rank, size;
    long long N = 10000000;          // sum 1 to N
    long long local_sum = 0, total_sum = 0;
    long long start, end;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Divide [1, N] as evenly as possible across 'size' processes.
    // First 'remainder' ranks get one extra number.
    long long chunk = N / size;
    long long remainder = N % size;

    if (rank < remainder) {
        start = rank * (chunk + 1) + 1;
        end = start + chunk;
    } else {
        start = rank * chunk + remainder + 1;
        end = start + chunk - 1;
    }

    // Each process sums its own slice
    for (long long i = start; i <= end; i++) {
        local_sum += i;
    }

    printf("Process %d: summing %lld..%lld, local sum = %lld\n",
           rank, start, end, local_sum);

    // Combine all local sums into total_sum on rank 0
    MPI_Reduce(&local_sum, &total_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("--------------------------------------------------\n");
        printf("Total sum of 1 to %lld = %lld\n", N, total_sum);
    }

    MPI_Finalize();
    return 0;
}