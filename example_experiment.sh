#!/bin/bash
# init
INIT=1
if [ $INIT == 1 ]; then
    bash init.sh || exit
    cd branch_and_reduce ||exit
    bash compile_withcmake.sh || exit
    cd ..
fi

# example_experiment.sh
results="results.csv"
rm -v $results
echo "graph,2pack,,red2pack elaborated S,,red2pack core,,gen2pack,,Apx-2p+Imp2p," >> $results
echo ",S,t,S,t,S,t,S,t,S,t" >> $results # solution + time found

# Usage: bash run_experiment.sh <path_to_graph_filename> <use_genpack:1:0> <use_apx2p:1:0>

# Erdos-Renyi graph from https://github.com/trejoel/Gene2Pack/tree/amcs
bash run_experiment.sh graphs/example/aGraphErdos40-1 1 0 >> $results
echo "Finished aGraphErdos40-1"
# Cactus graph from https://github.com/trejoel/Gene2Pack/tree/amcs
bash run_experiment.sh graphs/example/cac100 1 0 >> $results
echo "Finished cac100"
# Outer planar graph from https://github.com/trejoel/Approximate2Packing
bash run_experiment.sh graphs/example/Outerplanar500_1 0 1 >> $results
echo "Finished Outerplanar500_1"
# Social graph from https://dimacs10.github.io/archive/clustering.shtml
bash run_experiment.sh graphs/example/lesmis 0 0 >> $results
echo "Finished lesmis"
bash run_experiment.sh graphs/example/cond-mat-2005 0 0 >> $results
echo "Finished cond-mat-2005"