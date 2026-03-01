#!/bin/bash

##########
# Generate files with jobs for a benchmark configuration
##########

echo "Generating jobs for a benchmark"

benchmarkcfg=$1

source $benchmarkcfg

echo "timelimit: $TIME_LIMIT"
echo "seeds: ${SEEDS[@]}"
echo "configs: ${CONFIGS[@]}"
echo "instances-file: $INSTANCES_FILE (counting: $(cat $INSTANCES_FILE |wc -l |xargs))"

function call_job_creator() {
	local job_creator=$1
	shift
	$job_creator "$@"
}

job=$(date '+%Y%m%d%H%M%S')
OUT_DIR="out-$(basename $BENCHMARK_NAME)-$job"
JOB_FILE="jobs-$(basename $BENCHMARK_NAME)-$job.txt"
SHUF_JOB_FILE="shuf-jobs-$(basename $BENCHMARK_NAME)-$job.txt"
mkdir $OUT_DIR

for cfg in "${CONFIGS[@]}"; do
	mkdir $OUT_DIR/$cfg
done

while read instance; do
	for seed in "${SEEDS[@]}"; do

		i=0
		for cfg in "${CONFIGS[@]}"; do
			job_creator=${CONFIG_CREATORS[i]}

			call_job_creator "$job_creator" "$instance" "$cfg" "$OUT_DIR" "$seed" "$TIME_LIMIT" >> $JOB_FILE
			i=$((i+1))	
		done

	done
done <$INSTANCES_FILE

export OUT_DIR
export JOB_FILE

echo "############## BENCHMARK ##############"
echo "# BENCHMARK_NAME: $BENCHMARK_NAME"
echo "# JOB_FILE: $JOB_FILE (counting $(cat $JOB_FILE |wc -l |xargs))"
echo "# OUT_DIR: $OUT_DIR "
echo "# TASKS (cpus): $TASKS"
echo "# GB_PER_TASK: $GB_PER_TASK"
echo "# run on a memory machine: exclusive parallel -j $TASKS --memfree $((2*$GB_PER_TASK))g --retries 100000 --delay 1 --progress --eta ./job_wrapper.sh $GB_PER_TASK $TIME_LIMIT :::: $JOB_FILE"
echo "# or sequentially: cat $JOB_FILE | xargs -I % sh -c %"
echo "#######################################"
