#!/bin/bash
# Usage:  ./run_bench.sh ["1 2 4 8"] [reps]
# Optional env var MPI_EXTRA for extra mpirun flags, e.g.
#   MPI_EXTRA="--oversubscribe" ./run_bench.sh "1 2 4 8 16" 5
PROCS=${1:-"1 2 4 8"}
REPS=${2:-5}
EXTRA=${MPI_EXTRA:-""}

mpicc -O0 -o ex2_sum ex2_sum.c       || exit 1   # -O0: stop compiler turning the loop into a closed-form formula
mpicc -O2 -o ex3_pi  ex3_pi.c -lm    || exit 1

echo "exercise,procs,run,time" > results.csv
for ex in ex2_sum ex3_pi; do
  for p in $PROCS; do
    for r in $(seq 1 "$REPS"); do
      t=$(mpirun $EXTRA -np "$p" ./$ex | grep "Time (max rank)" | awk '{print $(NF-1)}')
      echo "$ex,$p,$r,$t" >> results.csv
      echo "$ex  np=$p  run=$r  ${t}s"
    done
  done
done
echo "Saved results.csv"