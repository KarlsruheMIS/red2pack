#!/bin/bash

mkdir -p graphs
mkdir -p graphs/erdos_graphs
mkdir -p graphs/erdos_graphs_gml
mkdir -p graphs/cactus_graphs
mkdir -p graphs/cactus_graphs_gml
mkdir -p graphs/planar_graphs
mkdir -p graphs/planar_graphs_gml

SKIP_GIT=0
if [ ! -d .git ]; then
  # skip git, assert submodules were downloaded
  SKIP_GIT=1
  echo "Found no git Repository. Skipping initialization of sub modules. Please, provide dependencies by yourself."
fi

# initialize and update submodules
if [ $SKIP_GIT -eq 0 ]; then
git submodule update --init --recursive --force
fi

# translate Erdos-Renyi + Cactus instances to metis format
echo "Preparing Erdos-Renyi + Cactus graphs"
cd graphs/Gene2Pack || exit
if [ $SKIP_GIT -eq 0 ]; then
  git checkout amcs
fi
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
cd ../

# translate Planar instances to metis format
echo "Preparing Outerplanar graphs"
cd Approximate2Packing || exit
find Instances_*.zip|xargs -I % unzip %
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






