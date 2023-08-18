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
mkdir deploy
if [[ "$unamestr" == "Linux" ]]; then
	make CPLEX_DIRECTORY=${CPLEX_ROOT_DIR}
	mv modified_source deploy/
	rm modified_source.o
fi

if [[ "$unamestr" == "Darwin" ]]; then
rm -rf build
mkdir build
cd build
  cmake ../ -DCMAKE_C_COMPILER=$(which clang)  -DCMAKE_CXX_COMPILER=$(which clang++) -DCMAKE_BUILD_TYPE=Release -DCMAKE_ENABLE_TESTING=False -DCPLEX_ROOT_DIR=$CPLEX_ROOT_DIR
make -j $NCORES
cd ..
cp ./build/apx-2p deploy/

rm -rf build
fi


