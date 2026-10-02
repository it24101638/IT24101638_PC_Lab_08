#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <mpi.h>

#define DEFAULT_N 10000000ULL
#define TAG_HITS  0

static uint64_t splitmix64(uint64_t x)
{
    x += 0x9E3779B97F4A7C15ULL;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
    return x ^ (x >> 31);
}

static inline double next_double(uint64_t *s)
{
    *s ^= *s >> 12;
    *s ^= *s << 25;
    *s ^= *s >> 27;
    return ((*s * 2685821657736338717ULL) >> 11) * (1.0 / 9007199254740992.0);
}

int main(int argc, char *argv[])
{
    int rank, size;
    unsigned long long total = DEFAULT_N;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc > 1)
        total = strtoull(argv[1], NULL, 10);

    unsigned long long local_n = total / size + ((unsigned long long)rank < total % size ? 1 : 0);

    uint64_t state = splitmix64(2024ULL + (uint64_t)rank);   // same seeds as original Ex3
    if (state == 0) state = 1;

    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();

    unsigned long long local_hits = 0;
    for (unsigned long long i = 0; i < local_n; i++) {
        double x = next_double(&state);
        double y = next_double(&state);
        if (x * x + y * y <= 1.0)
            local_hits++;
    }

    unsigned long long global_hits = local_hits;
    int *arrival = (int *) malloc(size * sizeof(int));   // order in which ranks' results arrive
    int n_recv = 0;

    if (rank != 0) {
        // Workers send their count to rank 0
        MPI_Send(&local_hits, 1, MPI_UNSIGNED_LONG_LONG, 0, TAG_HITS, MPI_COMM_WORLD);
    } else {
        // Rank 0 accepts results from ANY rank, in whatever order they arrive
        for (int i = 1; i < size; i++) {
            unsigned long long incoming;
            MPI_Status status;
            MPI_Recv(&incoming, 1, MPI_UNSIGNED_LONG_LONG,
                     MPI_ANY_SOURCE,            // <-- changed from a fixed source
                     TAG_HITS, MPI_COMM_WORLD, &status);
            global_hits += incoming;
            arrival[n_recv++] = status.MPI_SOURCE;   // who actually sent it
        }
    }

    double local_time = MPI_Wtime() - t0, max_time = 0.0;
    MPI_Reduce(&local_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double pi = 4.0 * (double)global_hits / (double)total;
        printf("Processes      : %d\n", size);
        printf("Total points   : %llu\n", total);
        printf("Points in circle: %llu\n", global_hits);
        printf("Estimated Pi   : %.8f\n", pi);
        printf("Actual Pi      : %.8f\n", M_PI);
        printf("Error          : %.8f\n", fabs(pi - M_PI));
        printf("Time (max rank): %.6f s\n", max_time);
        printf("Arrival order  :");
        for (int i = 0; i < n_recv; i++) printf(" %d", arrival[i]);
        printf("\n");
    }

    free(arrival);
    MPI_Finalize();
    return 0;
}