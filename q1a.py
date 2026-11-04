from os import listdir
from os.path import isfile, join
import numpy as np

folder = 'outputs/q1'

for f in listdir(folder):
    g = []
    r = []
    if f.startswith('greedy'):
        with open(folder + '/' + f) as file:
            for line in file:
                g.append(float(line.strip()))
        with open(folder + '/' + f.replace('greedy', 'random')) as file:
            for line in file:
                r.append(float(line.strip()))

        g = np.array(g)
        r = np.array(r)

        diff = g - r
        print(f, len(diff[diff > 0]), len(diff[diff < 0]))
        print(g.mean(), r.mean(), diff.mean())
