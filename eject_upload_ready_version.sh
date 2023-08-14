#!/bin/bash
trap "echo exiting...; exit" SIGHUP SIGINT SIGTERM
echo "This transforms this repository to an upload ready version and cannot be undone. If you want to proceed, hit a key. Otherwise exit with Ctrl+C."
read var

echo "Preparing upload ready version."

git stash
git submodule update --init --recursive

if [ ! -d branch_and_reduce/extern/KaMIS ]; then
  echo "KaMIS not downloaded!"
  exit 1
fi

if [ ! -d graphs/Gene2Pack ]; then
  echo "Gene2Pack not downloaded!"
  exit 1
else
  cd graphs/Gene2Pack
  git checkout amcs
  cd ../../
fi

if [ ! -d graphs/Approximate2Packing ]; then
  echo "Approximate2Packing not downloaded!"
  exit 1
fi

rm data_table.py
rm eval_results.py
rm gen_algo.py
rm gen_jobs*
rm -rf .git
rm eject_upload_ready_version.sh

echo "Upload ready version transformation was successful!"
echo "Now run:"
PWD=$(pwd)
echo "cd .. && tar -czvf ${PWD##*/}-$(date '+%Y%m%d%H%M%S').tar.gz ${PWD##*/} && rm -rf ${PWD##*/}"

