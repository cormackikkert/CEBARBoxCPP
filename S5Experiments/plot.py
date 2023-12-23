import sys
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker

solvers = ["CEGAR", "Cheetah", "S52SAT"]

def plot(timeouts, counts, total, name):
    fig, ax = plt.subplots()
    colours = ["#B931FC", "#5CD2E6", "#FF9800"]

    for i, solver in enumerate(solvers):
        ax.plot(timeouts, counts[solver], label=solver, c = colours[i])
    # ax.plot(timeouts, [x[0] for x in counts], label="CEGAR", c = "#B931FC")
    # ax.plot(timeouts, [x[1] for x in counts], label="Cheetah", c = "#5CD2E6")

    ax.plot(timeouts, [total for x in timeouts], label="Total", c = "#FF0000", linestyle="--")
    ax.set_xlabel("Given Time (in s, log scale)")

    ax.set_xscale("log", base=2)
    ax.set_xticks(timeouts)
    ax.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())

    ax.set_ylabel("Number of instances solved")
    ax.legend()
    ax.grid(linestyle="--", color="grey", linewidth=0.5)

    plt.savefig(f"plots/{name}.png")
    plt.show()

def make_counts(times_df, timeouts, solvers):
    counts = {solver: [] for solver in solvers}
    
    for solver in solvers:
        for timeout in timeouts:
            counts[solver].append(((times_df[solver] <= timeout) & (times_df[solver] != -1)).sum())

    return counts

if __name__ == "__main__":
    benchmark_name = sys.argv[1]
    times_df = pd.read_csv(f"./results/{benchmark_name}/times.csv")

    counts = make_counts(times_df, [0.25, 0.5, 1, 2, 4], solvers)
    total = len(times_df)

    plot([0.25, 0.5, 1, 2, 4], counts, total, benchmark_name)