#!/bin/bash
if (( $# != 2 )); then
  >&2 echo "Usage: $0 <buildtype:Release/Debug> <build tests:True/False>  Default: buildtype:Release tests:False"
  buildtype=Release
  testing=False
else
buildtype=$1 # Release or Debug
testing=$2 # True or False
fi

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
rm -rf build
mkdir build
cd build
if [[ "$unamestr" == "Linux" ]]; then
  cmake ../ -DCMAKE_C_COMPILER=$(which gcc) -DCMAKE_CXX_COMPILER=$(which g++) -DCMAKE_BUILD_TYPE=$buildtype -DCMAKE_ENABLE_TESTING=$testing
fi

if [[ "$unamestr" == "Darwin" ]]; then
  cmake ../ -DCMAKE_C_COMPILER=/usr/local/bin/gcc-13  -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-13  -DCMAKE_BUILD_TYPE=$buildtype -DCMAKE_ENABLE_TESTING=$testing
fi

make -j $NCORES

cmake --install .

cd ..
#rm -rf build
