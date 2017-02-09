#!/usr/bin/python

import sys
import os
import getopt
import subprocess
from subprocess import call
import re
import numpy as np
import matplotlib.pyplot as plt
from operator import itemgetter

nodelist = []
NODE_CNT = 153
dijhops = []
airhops = []
files = []
#comblist = []

def readfiles():
    color = 'blue'
    maxlist = [0, 0]

    bar_width = 0.35
    opacity = 0.4
    error_config = {'ecolor': '0.3'}
    index = np.arange(NODE_CNT)

    valuelist = []
    airlist = []
    dijlist = []
    comblist = []

    dijfile = files[0]
    airfile = files[1]
    #dijdelay = []
    #airdelay = []

    with open(dijfile, "r") as f:
        next(f);
        for line in f:
            values = []
            words = line.split()
            if words[4] != "FullDelay":
                continue;
            node = int(words[1].strip('Node'))
            #delay = float(words[6])
            hops = int(words[8])
            values.append(dijfile)
            values.append(node)
            #values.append(delay)
            values.append(hops)
            dijlist.append(values)

    with open(airfile, "r") as f:
        next(f);
        for line in f:
            values = []
            words = line.split()
            if words[4] != "FullDelay":
                continue;
            node = int(words[1].strip('Node'))
            #delay = float(words[6])
            hops = int(words[8])
            values.append(airfile)
            values.append(node)
            #values.append(delay)
            values.append(hops)
            airlist.append(values)

    for dij in dijlist:
        for air in airlist:
            comb = []
            if air[1] == dij[1]:
                comb = dij + air
                comblist.append(comb)
                #print comb
    return comblist

def sortlist(comblist):
    comblist = sorted(comblist,key=itemgetter(1))
    return comblist

def getxy(comblist):
    for comb in comblist:
        nodelist.append(comb[1])
        #dijdelay.append(comb[2])
        #airdelay.append(comb[6])
        dijhops.append(comb[2])
        airhops.append(comb[5])
        #print comb
#        return airhops, dijhops

def computemean():
    # compute mean and variance
    airmean = np.mean(airhops)
    airvariance = np.var(airhops)
    dijmean = np.mean(dijhops)
    dijvariance = np.var(dijhops)

    statres = str(dest) + "\t" + str(airmean) + "\t"  +\
            str(airvariance) + "\t" + str(dijmean) +  "\t" +\
            str(dijvariance) + "\n"


    with open("scratch/mean_variance.txt", "a") as statfile:
        statfile.write(statres)



airmean = []
airvariance = []
dijmean = []
dijvariance = []
statfile = "scratch/mean_variance.txt"

def main(total):
    p = subprocess.Popen("rm scratch/mean_variance.txt", shell=True)
    p.wait();

    for i in range(0, total):
        runsingle(i)

    with open(statfile, "r") as f:
        for line in f:
            values = []
            words = line.split()
            airmean.append(float(words[1]))
            airvariance.append(float(words[2]))
            dijmean.append(float(words[3]))
            dijvariance.append(float(words[4]))

def creategraph(total):
    y_pos = np.arange(len(nodelist))

    fig, ax = plt.subplots()

    index = np.arange(26)
    bar_width = 0.35

    opacity = 0.4
    error_config = {'ecolor': '0.3'}
    '''
    rects1 = plt.bar(index, dijhops, bar_width,
             alpha=opacity, color='b', yerr=0, error_kw=error_config, label='Dijkstra\'s')

    rects2 = plt.bar(index + bar_width, airhops, bar_width,
             alpha=opacity, color='r', yerr=0, error_kw=error_config, label='Air')
    '''
    print airmean
    print airvariance
    plt.errorbar(index, airmean, xerr=0, yerr=airvariance)
    plt.errorbar(index, dijmean, xerr=0, yerr=dijvariance)

    plt.xlabel('Nodes')
    plt.ylabel('Hop Count')
    plt.title('Hop count of each node to the destination')
    '''hide the x axix labels'''
    #frame1 = plt.gca()
    #frame1.axes.get_xaxis().set_ticks([])
    #ax.set_xticks(nodeids)
    #ax.set_xticklabels(nodeids)
    #rects = ax.patches
    #for rect, label in zip(rects, nodeids):
    #    height = rect.get_height()
    #    ax.text(rect.get_x() + rect.get_width()/2, height + 1, label, ha='center', va='bottom')

    plt.legend()

    #plt.tight_layout()
    plt.show()


def runsingle(dest):
    command = "scratch/nodes_graph.py " + str(dest)
    p = subprocess.Popen(command, shell=True)
    p.wait();

def getfiles(dest):
    files.append("scratch/subdir/statfiles/dij" + dest)
    files.append("scratch/subdir/statfiles/air" + dest)



if __name__ == "__main__":
    total = int(sys.argv[1])
    main(total);
    creategraph(total)
