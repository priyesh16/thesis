#!/usr/bin/python

import sys
import getopt
import subprocess
from subprocess import call
import re
import numpy as np
import matplotlib.pyplot as plt

def delayplot(files, outfile):
    color = 'blue'
    maxlist = [0, 0]
    NODE_CNT = 153

    bar_width = 0.35
    opacity = 0.4
    error_config = {'ecolor': '0.3'}
    index = np.arange(NODE_CNT)

    valuelist = []
    airlist = []
    dijlist = []
    comblist = []
    nodelist = []
    dijdelay = []
    airdelay = []
    dijhops = []
    airhops = []
    filename = mod_dij
    with open(filename, "r") as f:
        for line in f:
            values = []
            words = line.split()
            node = words[0]
            delay = float(words[1])
            hops = int(words[2])
            values.append(filename)
            values.append(node)
            values.append(delay)
            values.append(hops)
            dijlist.append(values)

    filename = mod_air
    with open(filename, "r") as f:
        for line in f:
            values = []
            words = line.split()
            node = words[0]
            delay = float(words[1])
            hops = int(words[2])
            values.append(filename)
            values.append(node)
            values.append(delay)
            values.append(hops)
            airlist.append(values)

    for dij in dijlist:
        for air in airlist:
            comb = []
            if air[1] == dij[1]:
                comb = dij + air
                comblist.append(comb)

    for comb in comblist:
        nodelist.append(comb[1])
        dijdelay.append(comb[2])
        airdelay.append(comb[6])
        dijhops.append(comb[3])
        airhops.append(comb[7])
        print comb

    print dijhops
    print airhops
    # Example data
    nodeids = [int(nodeid.strip('Node')) for nodeid in nodelist]

    print nodeids
    y_pos = np.arange(len(nodelist))
    n_groups = NODE_CNT

    fig, ax = plt.subplots()

    index = np.arange(n_groups)
    bar_width = 0.35

    opacity = 0.4
    error_config = {'ecolor': '0.3'}

    rects1 = plt.bar(index, dijhops, bar_width,
             alpha=opacity, color='b', yerr=0, error_kw=error_config, label='Dijkstra\'s')

    rects2 = plt.bar(index + bar_width, airhops, bar_width,
             alpha=opacity, color='r', yerr=0, error_kw=error_config, label='Air')

    plt.xlabel('Nodes')
    plt.ylabel('Hop Count')
    plt.title('Hop count of each node to the destination')
    '''hide the x axix labels'''
    frame1 = plt.gca()
    frame1.axes.get_xaxis().set_ticks([])
    #ax.set_xticks(nodeids)
    #ax.set_xticklabels(nodeids)
    #rects = ax.patches
    #for rect, label in zip(rects, nodeids):
    #    height = rect.get_height()
    #    ax.text(rect.get_x() + rect.get_width()/2, height + 1, label, ha='center', va='bottom')

    plt.legend()

    #plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    mod_air = "mod_air"
    mod_dij = "mod_dij"
    outfile = "test"
    files = [mod_air, mod_dij]
    delayplot(files, outfile)
