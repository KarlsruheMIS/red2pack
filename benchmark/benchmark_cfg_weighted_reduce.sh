#!/bin/bash
# Check if script is being sourced
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "Usage: ./generate_benchmark.sh ${1}"
    exit 1
fi

BENCHMARK_NAME="weighted_reduce"
TIME_LIMIT=$((4*60*60)) # seconds
SEEDS=( 1 2 )
INSTANCES_FILE="small_benchmark_instances.txt" # file listing file paths to graphs
INSTANCE_NAMES_FILE="small_benchmark_instances_names.txt" # file listing filenames
BIN_DIR="deploy"

export TASKS=60 # tasks in parallel
export GB_PER_TASK=200 # GB

# we name option that we toggle off (f)/on (n)
# we add the exact configuration to obtain optimal solutions
declare -a CONFIGS=(
	"strong"
	"fast"
	"full"
	"main"
	"transform"
)

# for each config, define how a job is built
declare -a CONFIG_CREATORS=(
	"c_reduce_strong"
	"c_reduce_fast"
	"c_reduce_full"
	"c_reduce_main"
	"c_transform"
)

############
# helpers
############
function c_base_cli() {
	local instance=$1
	local config=$2
	local outdir=$3
	local seed=$4
	local time_limit=$5
	local solver=$6
	shift 6

	echo "./${BIN_DIR}/$solver" \
		"$@"  \
		"--time_limit=$time_limit --seed=$seed $instance" \
	       	"> $outdir/$config/s${seed}-$(basename $instance).out 2>&1"
}


function c_reduce_cli() {
    local reduction_style=$1
    shift 1
    c_base_cli "$@" "redw2pack_reduce" "--reduction_style=$reduction_style"
}

############
# different reduction styles
############
function c_reduce_strong() {
    c_reduce_cli "strong" "$@"
}
function c_reduce_fast() {
    c_reduce_cli "fast" "$@"
}
function c_reduce_full() {
    c_reduce_cli "full" "$@"
}
function c_reduce_main() {
    c_reduce_cli "main" "$@"
}
############
# plain transformation
############
function c_transform() {
    c_base_cli "$@" "redw2pack_transform"
}


export TIME_LIMIT
export SEEDS
export CONFIGS
export CONFIG_CREATORS
export INSTANCES_FILE
export INSTANCE_NAMES_FILE