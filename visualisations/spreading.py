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

    ax.plot(cols['t'], cols['dead'], label='Dead')
    ax.plot(cols['t'], cols['infected'], label='Infected')
    ax.plot(cols['t'], cols['hospitalized'], label='Hospitalised')

    ax.set_xlabel('Time (days)')
    ax.set_ylabel('Population')

    ax.legend()

    fig.savefig("figures/dead.pdf")
