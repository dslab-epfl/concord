import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from math import exp

FIG_HEIGHT = 1.2
SINGLE_COL_WIDTH = 4

def produce_plot():
    fig, ax = plt.subplots(1, 1, figsize=(SINGLE_COL_WIDTH, FIG_HEIGHT))
    x = np.arange(0,1.01, 0.05)
    y1 = map(lambda x: exp(pow(x, 2))-0.8, x)
    y2 = map(lambda x: exp(pow(x+0.2, 20))-0.8, x)
    y3 = map(lambda x: exp(pow(x, 20))-0.8, x)

    ax.plot(x, list(y1), label="No tail latency optimizations")
    ax.plot(x, list(y2), label="Tail latency optimizations")
    ax.plot(x, list(y3), label="Target behavior")

    leg = ax.legend()
    leg.get_frame().set_linewidth(0.0)
    ax.legend(fontsize=7,frameon=False)
    ax.set_ylabel("Tail Latency",fontsize=8)
    ax.set_xlabel("Load",fontsize=8)
    ax.set_ylim([0,1.8])
    ax.tick_params(left = False, right = False , labelleft = False ,
                labelbottom = False, bottom = False)
    plt.savefig("fig1.eps")

matplotlib.rcParams["ps.useafm"] = True
produce_plot()
