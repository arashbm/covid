import sys
import csv
import numpy as np

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

    hospitalized = [np.nan]*24 + [82, 96, 108, 112, 134, 143, 137, 159, 160,
            180, 187, 209, 228, 231, 239, 244, 236, 235, 235, 230, 232, 226,
            215]

    icu = [np.nan]*24 + [22, 24, 32, 31, 41, 49, 56, 62, 65, 72, 73,
            76, 81, 83, 82, 82, 81, 80, 77, 74, 75, 75, 76]

    ax.plot(cols['t'], cols['dead'], label='Dead')
    ax.plot(range(len(deaths)), deaths, label='Dead (real)', ls='--')
    ax.plot(cols['t'], cols['infected'], label='Infected')
    ax.plot(cols['t'], cols['hospitalized'], label='Hospitalised')
    ax.plot(range(len(hospitalized)), hospitalized, label='Hospitalised (real)', ls='--')
    ax.plot(range(len(icu)), icu, label='ICU (real)', ls='--')
    ax.plot(cols['t'], cols['asymptomatic'], label='Asymptomatic')

    ax.set_xlabel('Time (days)')
    ax.set_ylabel('Population')

    ax.legend()

    fig.savefig(sys.stdout.buffer, format='pdf')
