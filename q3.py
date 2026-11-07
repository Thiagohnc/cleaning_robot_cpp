import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

df = pd.read_csv('outputs/q3/log')

df = df[df['passos'] >= 1e4]
df = df[df['passos'] <= 1e8]
x = df['passos'].unique()
y1 = df[df['gamma'] == 0.7]['tempo']
y2 = df[df['gamma'] == 0.99]['tempo']

plt.plot(x, y1, label=r'$\gamma=0.7$')
plt.plot(x, y2, label=r'$\gamma=0.99$')
plt.xlabel('Iterações', fontsize=16)
plt.ylabel('Tempo (s)', fontsize=16)
plt.legend()
plt.title('Tempo de execução: Q-Learning', fontsize=18)
plt.tick_params(axis='both', labelsize=14)
plt.tight_layout()
plt.xscale('log')
plt.savefig(f'outputs/q3/tempo.pdf')
plt.close()
