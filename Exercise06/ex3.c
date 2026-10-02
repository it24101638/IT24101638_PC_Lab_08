/*
 * Monte Carlo estimation of Pi using MPI
 *
 * Idea: throw random points into the unit square [0,1) x [0,1).
 * Fraction landing inside the quarter circle (x^2 + y^2 <= 1) ~ Pi/4.
 *
 * Compile: mpicc -O2 -o mpi_pi mpi_pi.c
 * Run:     mpirun -np 4 ./mpi_pi            (10,000,000 points)
 *          mpirun -np 4 ./mpi_pi 50000000   (custom total)
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <mpi.h>

#define DEFAULT_N 10000000ULL

/* splitmix64: turns (seed + rank) into a well-mixed, distinct starting state */
static uint64_t splitmix64(uint64_t x)
{
    x += 0x9E3779B97F4A7C15ULL;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
    return x ^ (x >> 31);
}

/* xorshift64*: fast, per-process generator returning a double in [0,1) */
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

    /* Split work: every rank gets total/size, first (total % size) ranks get +1 */
    unsigned long long local_n = total / size + ((unsigned long long)rank < total % size ? 1 : 0);

    /* Distinct seed per rank so streams are not identical */
    uint64_t state = splitmix64(2024ULL + (uint64_t)rank);
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

    unsigned long long global_hits = 0;
    MPI_Reduce(&local_hits, &global_hits, 1, MPI_UNSIGNED_LONG_LONG,
               MPI_SUM, 0, MPI_COMM_WORLD);

    double t1 = MPI_Wtime();
    double local_time = t1 - t0, max_time = 0.0;
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
    }

    MPI_Finalize();
    return 0;
}