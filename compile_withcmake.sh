#!/bin/bash
if (( $# != 1 )); then
  >&2 echo "Usage: $0 <buildtype:Release/Debug> Default: buildtype:Release"
  buildtype=Release
else
buildtype=$1 # Release or Debug
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
  cmake ../ -DCMAKE_C_COMPILER=$(which gcc) -DCMAKE_CXX_COMPILER=$(which g++) -DCMAKE_BUILD_TYPE=$buildtype
elif [[ "$unamestr" == "Darwin" ]]; then
  cmake ../ -DCMAKE_C_COMPILER=$(which gcc-14) -DCMAKE_CXX_COMPILER=$(which g++-14)  -DCMAKE_BUILD_TYPE=$buildtype
fi

make -j $((NCORES/2))

cmake --install .

cd ..
#rm -rf build
