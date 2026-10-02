import csv, statistics
from collections import defaultdict
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

TITLES = {"ex2_sum": "Exercise 2: Sum 1..10,000,000",
          "ex3_pi":  "Exercise 3: Monte Carlo Pi (10,000,000 points)"}

# results[exercise][procs] -> list of run times
results = defaultdict(lambda: defaultdict(list))
with open("results.csv") as f:
    for row in csv.DictReader(f):
        results[row["exercise"]][int(row["procs"])].append(float(row["time"]))

summary = {}   # exercise -> (procs, median_times, speedups)
print(f"{'Exercise':<10}{'Procs':>6}{'Time (s)':>12}{'Speedup':>10}{'Efficiency':>12}")
for ex, d in results.items():
    procs = sorted(d)
    times = [statistics.median(d[p]) for p in procs]
    t1 = times[procs.index(1)] if 1 in procs else times[0]
    speed = [t1 / t for t in times]
    summary[ex] = (procs, times, speed)
    for p, t, s in zip(procs, times, speed):
        print(f"{ex:<10}{p:>6}{t:>12.6f}{s:>10.2f}{s/p:>12.2f}")

def make_fig(kind, fname):
    fig, axes = plt.subplots(1, len(summary), figsize=(6 * len(summary), 4.5))
    if len(summary) == 1:
        axes = [axes]
    for ax, (ex, (procs, times, speed)) in zip(axes, summary.items()):
        if kind == "time":
            ax.plot(procs, times, "o-", color="tab:blue")
            ax.set_ylabel("Time (s)")
        else:
            ax.plot(procs, speed, "o-", color="tab:green", label="Measured")
            ax.plot(procs, procs, "--", color="gray", label="Ideal (linear)")
            ax.set_ylabel("Speedup  (T1 / Tp)")
            ax.legend()
        ax.set_xlabel("Number of processors")
        ax.set_xticks(procs)
        ax.set_title(TITLES.get(ex, ex))
        ax.grid(alpha=0.3)
    fig.tight_layout()
    fig.savefig(fname, dpi=150)

make_fig("time", "time_vs_processors.png")
make_fig("speedup", "speedup.png")
print("Saved time_vs_processors.png and speedup.png")