#!/bin/bash

# set cplex root dir
# e.g. /Applications/CPLEX_Studio2211
CPLEX_ROOT_DIR=$1

NCORES=4
unamestr=$(uname)
if [[ "$unamestr" == "Linux" ]]; then
  NCORES=$(grep -c ^processor /proc/cpuinfo)
fi

if [[ "$unamestr" == "Darwin" ]]; then
  NCORES=$(sysctl -n hw.ncpu)
fi

rm -rf deploy
rm -rf build
mkdir build
cd build

if [[ "$unamestr" == "Linux" ]]; then
  cmake ../ -DCMAKE_C_COMPILER=$(which gcc) -DCMAKE_CXX_COMPILER=$(which g++) -DCMAKE_BUILD_TYPE=Release -DCMAKE_ENABLE_TESTING=False -DCPLEX_ROOT_DIR=$CPLEX_ROOT_DIR
fi

if [[ "$unamestr" == "Darwin" ]]; then
  cmake ../ -DCMAKE_C_COMPILER=$(which clang)  -DCMAKE_CXX_COMPILER=$(which clang++) -DCMAKE_BUILD_TYPE=Release -DCMAKE_ENABLE_TESTING=False -DCPLEX_ROOT_DIR=$CPLEX_ROOT_DIR
fi

make -j $NCORES
cd ..

mkdir deploy
cp ./build/apx-2p deploy/

rm -rf build
