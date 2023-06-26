#!/bin/bash

mkdir -p graphs
mkdir -p graphs/erdos_graphs
mkdir -p graphs/erdos_graphs_gml
mkdir -p graphs/cactus_graphs
mkdir -p graphs/cactus_graphs_gml

# initialize and update submodules
git submodule update --init --recursive

# download and translate Erdos instances
cd graphs/Gene2Pack || exit
git checkout amcs
unzip Graphs.zip
cd Graphs/Erdos-Renyi-Graphs || exit
for D in *; do
    if [ -d "${D}" ]; then
        cd "${D}" || exit
        for F in *; do
          python3 "../../../../../graph_gml_to_dimacs.py" "${F}" "../../../../erdos_graphs/"
          cp "${F}" "../../../../erdos_graphs_gml/"
        done
        cd ../
    fi
done
cd ../Cactus-Graphs || exit
for F in *; do
  python3 "../../../../graph_gml_to_dimacs.py" "${F}" "../../../cactus_graphs/"
  cp "${F}" "../../../cactus_graphs_gml/"
done
cd ../../
rm -r Graphs