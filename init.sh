#!/bin/bash

mkdir -p graphs
mkdir -p graphs/erdos_graphs
mkdir -p graphs/erdos_graphs_gml
mkdir -p graphs/cactus_graphs
mkdir -p graphs/cactus_graphs_gml
mkdir -p graphs/planar_graphs
mkdir -p graphs/planar_graphs_gml

# initialize and update submodules
git submodule update --init --recursive --force

# translate Erdos-Renyi + Cactus instances to metis format
echo "Preparing Erdos-Renyi + Cactus graphs"
cd graphs/Gene2Pack || exit
git checkout amcs
tar -xvf Graphs.zip
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
cd ../

# translate Planar instances to metis format
echo "Preparing Outerplanar graphs"
cd Approximate2Packing || exit
find Instances_*.zip|xargs -I % tar -xvf %
rm -r LastVersion
for D in *; do
    if [ -d "${D}" ]; then
        cd "${D}" || exit
        for F in *.gml; do
          python3 "../../../graph_gml_to_dimacs.py" "${F}" "../../planar_graphs/"
          cp "${F}" "../../planar_graphs_gml/"
        done
        cd ../
    fi
done
find Instances_* |grep -v ".zip"|xargs -I % rm -r %
cd ../../






