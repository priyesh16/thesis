#!/bin/bash -x

END=4
NODE_CNT=3
for i in $(seq 1 $END);
do
    for j in $(seq 1 $NODE_CNT);
    do
        ../waf "--run=ndn_air dij $j $i";
        ../waf "--run=ndn_air air $j $i";
        cat outfile_dij$j$i | grep Full | awk '{print $2,$6,$NF}' > mod_dij$j$i;
        cat outfile_air$j$i | grep Full | awk '{print $2,$6,$NF}' > mod_air$j$i;
    done;
done
