#!/bin/bash

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
cmake ../ -DCMAKE_C_COMPILER=$(which gcc) -DCMAKE_CXX_COMPILER=$(which g++) -DCMAKE_BUILD_TYPE=Release -DCMAKE_ENABLE_TESTING=False
make -j $NCORES
cd ..

mkdir deploy
cp ./build/m2s_branch_and_reduce deploy/

rm -rf build
