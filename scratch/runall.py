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


def sortlist(comblist):
    comblist = sorted(comblist,key=itemgetter(1))
    return comblist

airmean = []
airvariance = []
dijmean = []
dijvariance = []
statfile = "scratch/mean_variance.txt"

def main(total):
    #p = subprocess.Popen("rm scratch/mean_variance.txt", shell=True)
    #p.wait();

    #for i in range(0, total):
        #runsingle(i)
    matrix = []
    with open(statfile, "r") as f:
        for line in f:
            words = line.split()
            values = [float(words[0]), float(words[1]),
                        float(words[2]), float(words[3]), float(words[4])]
            matrix.append(values)
            '''
            airmean.append(float(words[1]))
            airvariance.append(float(words[2]))
            dijmean.append(float(words[3]))
            dijvariance.append(float(words[4]))
            '''

    for row in matrix:
        print row

    sort = matrix
    #sort = sortlist(matrix)

    for row in sort:
        print row
        airmean.append(row[1])
        airvariance.append(row[2])
        dijmean.append(row[3])
        dijvariance.append(row[4])

def creategraph(total):
    y_pos = np.arange(len(nodelist))

    fig, ax = plt.subplots()

    index = np.arange(total)
    bar_width = 0.35

    opacity = 0.4
    error_config = {'ecolor': '0.3'}
    '''
    rects1 = plt.bar(index, dijhops, bar_width,
             alpha=opacity, color='b', yerr=0, error_kw=error_config, label='Dijkstra\'s')

    rects2 = plt.bar(index + bar_width, airhops, bar_width,
             alpha=opacity, color='r', yerr=0, error_kw=error_config, label='Air')
    '''
    dijsd = np.sqrt(dijvariance)

    airsd = np.sqrt(airvariance)
    axes = plt.gca()
    axes.set_ylim([0,10])

    plt.errorbar(index, airmean, xerr=0, yerr=airsd)
    plt.errorbar(index, dijmean, xerr=0, yerr=dijsd)
    #plt.errorbar(index, dijmean, xerr=0, yerr=dijvariance)

    plt.xlabel('Nodes')
    plt.ylabel('Hop Count')
    plt.title('Mean and Standard Deviation of hop count from every node to each other')
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
    #plt.show()
    #fig = plt.figure()
    plt.savefig("scratch/mean_variance.png", bbox_inches='tight')
    plt.close(fig)

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
