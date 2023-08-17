#!/bin/bash

results="results.csv"
time_limit=10 # seconds

# init
INIT=1
USE_COMPETITOR_APX2P_IM2P=1
CPLEX_ROOT_DIR="/Applications/CPLEX_Studio2211" # provide cplex root dir if you want to test the competitor algorithm Apx-2p+Im-2p
if [ $INIT -eq 1 ]; then
    if [ -d .git ]; then
      # initialize and update submodules
      git submodule update --init --recursive --force
    fi
    cd branch_and_reduce ||exit
    bash compile_withcmake.sh || exit
    cd ..
    cp heuristic/mis_permutation_online.cpp heuristic/extern/KaMIS/lib/data_structure
    cd heuristic ||exit
    bash compile_withcmake.sh || exit
    cd ..
    cd competitor/Gene2Pack ||exit
    bash init.sh || exit
    cd ../../
    if [ $USE_COMPETITOR_APX2P_IM2P == 1 ]; then
      cd competitor/Approximate2Packing ||exit
      bash compile_withcmake.sh $CPLEX_ROOT_DIR ||exit
      cd ../../
    fi
fi

# example_experiment.sh

rm $results
echo "graph,2pack_bnr,,,,,red2pack_bnr core,,,,,red2pack_bnr elaborated,,,,,red2pack_bnr heuristic elaborated,,,,gen2pack,,Apx-2p+Imp2p," >> $results
echo ",S,t,t_p,n,m,S,t,t_p,n,m,S,t,t_p,n,m,S,t,n,m,S,t,S,t" >> $results # solution + time found

# Usage: bash run_experiment.sh <path_to_graph_filename> <use_genpack:1:0> <use_apx2p:1:0> <time_limit>

# Erdos-Renyi graph from https://github.com/trejoel/Gene2Pack/tree/amcs
bash run_experiment.sh graphs/example/aGraphErdos40-8 1 0 $time_limit >> $results
echo "Finished aGraphErdos40-8"
# Cactus graph from https://github.com/trejoel/Gene2Pack/tree/amcs
bash run_experiment.sh graphs/example/cac100 1 0 $time_limit>> $results
echo "Finished cac100"
# Outer planar graph from https://github.com/trejoel/Approximate2Packing
bash run_experiment.sh graphs/example/Outerplanar500_1 0 $USE_COMPETITOR_APX2P_IM2P $time_limit >> $results
echo "Finished Outerplanar500_1"
# Social graph from https://dimacs10.github.io/archive/clustering.shtml
bash run_experiment.sh graphs/example/lesmis 0 0 $time_limit >> $results
echo "Finished lesmis"
bash run_experiment.sh graphs/example/cond-mat-2005 0 0 $time_limit >> $results
echo "Finished cond-mat-2005"
