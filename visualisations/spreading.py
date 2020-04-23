import sys
import csv

import matplotlib.pyplot as plt

if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise "specify an input file"

    cols = {}
    with open(sys.argv[1], newline='') as csvfile:
        for row in csv.DictReader(csvfile, delimiter=' '):
            for k, v in row.items():
                if k not in cols:
                    cols[k] = []
                cols[k].append(float(v))

    fig, ax = plt.subplots()

    deaths = [0]*20 + [1, 1, 1, 1, 3, 4, 7, 9, 11, 13, 17, 17, 19, 20, 25, 28,
            27, 34, 40, 42, 48, 49, 56, 59, 64, 72]

    ax.plot(cols['t'], cols['dead'], label='Dead')
    ax.plot(range(len(deaths)), deaths, label='Dead (real)', ls='--')
    ax.plot(cols['t'], cols['infected'], label='Infected')
    ax.plot(cols['t'], cols['hospitalized'], label='Hospitalised')
    ax.plot(cols['t'], cols['asymptomatic'], label='Asymptomatic')

    ax.set_xlabel('Time (days)')
    ax.set_ylabel('Population')

    ax.legend()

    fig.savefig(sys.stdout.buffer, format='pdf')
