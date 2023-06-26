#!/bin/bash

mkdir -p graphs
mkdir -p graphs/erdos_graphs
mkdir -p graphs/erdos_graphs_gml

# initialize and update submodules
git submodule init
git submodule update

# download and translate Erdos instances
cd graphs/Gene2Pack || exit
unzip commented_graphs.zip
cd commented_graphs || exit
for D in *; do
    if [ -d "${D}" ]; then
        cd "${D}" || exit
        for F in *; do
          python3 "../../../../graph_gml_to_dimacs.py" "${F}" "../../../erdos_graphs/"
          cp "${F}" "../../../erdos_graphs_gml/"
        done
        cd ../
    fi
done
cd ../
rm -r commented_graphs


