from os import listdir
from os.path import isfile, join
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

folder = 'outputs/q2'

def plot_by_k(_df, algoritmo, gamma):
    df = _df[_df['algoritmo'] == algoritmo]
    x = df[df['gamma'] == gamma]['k']
    y_t = df[df['gamma'] == gamma]['tempo_s']
    y_iter = df[df['gamma'] == gamma]['iteracoes']

    plt.plot(x, y_t)
    plt.xlabel('k', fontsize=16)
    plt.ylabel('Tempo (s)', fontsize=16)
    plt.title(fr'Tempo: {algoritmo} - $\gamma = {gamma}$', fontsize=18)
    plt.tick_params(axis='both', labelsize=14)
    plt.tight_layout()
    plt.savefig(f'{folder}/{algoritmo}_{gamma}_t.pdf')
    plt.close()

    plt.plot(x, y_iter)
    plt.xlabel('k', fontsize=16)
    plt.ylabel('Iterações', fontsize=16)
    plt.title(fr'Iterações: {algoritmo} - $\gamma = {gamma}$', fontsize=18)
    plt.tick_params(axis='both', labelsize=14)
    plt.tight_layout()
    plt.savefig(f'{folder}/{algoritmo}_{gamma}_iter.pdf')
    plt.close()


df = pd.read_csv(f"{folder}/log")

plot_by_k(df, 'ValueIteration', 0.7)
plot_by_k(df, 'PolicyIteration', 0.7)
plot_by_k(df, 'ValueIteration', 0.99)
plot_by_k(df, 'PolicyIteration', 0.99)
