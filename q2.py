from os import listdir
from os.path import isfile, join
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

folder = 'outputs/q2'

def plot_by_k(_df, algoritmo):
    df = _df[_df['algoritmo'] == algoritmo]
    x1 = df[df['gamma'] == 0.7]['k']
    y1_t = df[df['gamma'] == 0.7]['tempo_s']
    y1_iter = df[df['gamma'] == 0.7]['iteracoes']
    
    x2 = df[df['gamma'] == 0.99]['k']
    y2_t = df[df['gamma'] == 0.99]['tempo_s']
    y2_iter = df[df['gamma'] == 0.99]['iteracoes']

    plt.plot(x1, y1_t, label=r'$\gamma$ = 0.7')
    plt.plot(x2, y2_t, label=r'$\gamma$ = 0.99')
    plt.legend()
    plt.savefig(f'{folder}/{algoritmo}_t.pdf')
    plt.close()

    plt.plot(x1, y1_iter, label=r'$\gamma$ = 0.7')
    plt.plot(x2, y2_iter, label=r'$\gamma$ = 0.99')
    plt.legend()
    plt.savefig(f'{folder}/{algoritmo}_iter.pdf')
    plt.close()


df = pd.read_csv(f"{folder}/log")

plot_by_k(df, 'ValueIteration')
plot_by_k(df, 'PolicyIteration')
