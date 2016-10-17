#!/usr/bin/python

import sys
import getopt
import subprocess
from subprocess import call
import re
import numpy as np
import matplotlib.pyplot as plt

linesize=2000
linescale=linesize/10

def main(files, outfile):
    prevline = 0;
    max = 0;
    i = 0;

    for i in xrange(len(files)):
        filename = files[i]
        with open(filename, "r") as f:
            for line in f:
                words = line.split()
        print (pos, max)

def delayplot(files, outfile):
    color = 'blue'
    maxlist = [0, 0]
    NODE_CNT = 18

    '''
    bwplt.subplot(2,1,1)
    bwplt.xlabel("Loop Number * 2000")
    bwplt.ylabel("Timestamps")
    bwplt.xlim([0, 1])
    bwplt.ylim([0, 1.5])

    bwplt.subplot(2,1,2)
    bwplt.xlabel("CPU share")
    bwplt.ylabel("Cumulative Distribution")
    bwplt.xlim([0, 1])
    bwplt.ylim([0, 1])
    '''

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
        print comb
    print dijdelay
    print np.isscalar(dijdelay[0])
    # Example data
    y_pos = np.arange(len(nodelist))
    n_groups = NODE_CNT

    means_men = dijdelay
    means_women = airdelay

    fig, ax = plt.subplots()

    index = np.arange(n_groups)
    bar_width = 0.35

    opacity = 0.4
    error_config = {'ecolor': '0.3'}

    rects1 = plt.bar(index, dijdelay, bar_width,
             alpha=opacity, color='b', yerr=0, error_kw=error_config, label='Dijkstra\'s')

    rects2 = plt.bar(index + bar_width, airdelay, bar_width,
             alpha=opacity, color='r', yerr=0, error_kw=error_config, label='Air')

    plt.xlabel('Nodes')
    plt.ylabel('Latencies')
    plt.title('Latencies of each node with respect to destination')
    ax.set_xticks(index + bar_width)
    ax.set_xticklabels(nodelist)

    plt.legend()

    plt.tight_layout()
    plt.show()

'''
plt.subplots()
plt.barh(y_pos, dijdelay, color='b', xerr=0, align='center', alpha=0.4)
plt.barh(y_pos, airdelay, color='r', xerr=0, align='center', alpha=0.4)
plt.yticks(y_pos, nodelist)
plt.xlabel('Delay')
plt.title('How fast do you want to go today?')

plt.show()
plt.savefig("delaypng")
'''
'''
rects1 = plt.bar(index, dijdelay, bar_width, alpha=opacity,
color='b', yerr = 0, error_kw=error_config, label='Delay')
plt.xticks(index + bar_width, node)
#bwplt.figure(2)
#bwplt.scatter(hops, node, c=color, label=color, alpha=0.5)
'''
plt.show()
'''
bwplt.grid(True)
bwplt.figure(1)
bwplt.savefig("delaypng")
bwplt.figure(2)
bwplt.savefig("hopspng")
#bwplt.show()
'''


if __name__ == "__main__":
    mod_air = "mod_air"
    mod_dij = "mod_dij"
    outfile = "test"
    files = [mod_air, mod_dij]
    delayplot(files, outfile)
