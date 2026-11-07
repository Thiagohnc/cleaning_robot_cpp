from os import listdir
import os
from os.path import isfile, join
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

folder = 'outputs/q2/policy'

def read_policy(filepath):
    with open(filepath, 'r') as file:
        l = []
        for line in file:
            l.append(int(line.strip()))
    return np.array(l)


def count_diffs(policies):
    d = []
    for i in range(len(policies)):
        d.append([])
        for j in range(len(policies)):
            d[-1].append(100 * np.count_nonzero(policies[i] != policies[j])/606816)
    return d


for method in ('policyiter', 'valueiter'):
    for gamma in ('0.700000', '0.990000'):
        print(method, gamma)
        print("")
        policies = []
        for k in (0, 1, 2, 4, 6, 10):
            filepath = f'{folder}/{method}_{k}_{gamma}'
            policies.append(read_policy(filepath))
        print(count_diffs(policies))
        print("")
        print("")
        print("")
