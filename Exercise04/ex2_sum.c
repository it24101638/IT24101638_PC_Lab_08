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

    // ---- timing starts: all ranks begin together ----
    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();

    // Each process sums its own slice
    for (long long i = start; i <= end; i++) {
        local_sum += i;
    }

    // Combine all local sums into total_sum on rank 0
    MPI_Reduce(&local_sum, &total_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    double local_time = MPI_Wtime() - t0, max_time = 0.0;
    // ---- timing ends ----

    // Slowest rank decides the parallel run time
    MPI_Reduce(&local_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // Printing moved AFTER timing so I/O doesn't distort the measurement
    if (rank == 0) {
        printf("Processes      : %d\n", size);
        printf("Total sum of 1 to %lld = %lld\n", N, total_sum);
        printf("Time (max rank): %.6f s\n", max_time);
    }

    MPI_Finalize();
    return 0;
}