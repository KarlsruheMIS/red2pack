#!/usr/bin/env bash
# Benchmark script to run different (heuristic) MW2PS solvers on a fixed reduction style

# Check if script is being sourced
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "Usage: ./generate_benchmark.sh ${1}"
    exit 1
fi

reduction_style="fast" # set a reduction style
BENCHMARK_NAME="weighted_rnt_solvers_$reduction_style"
TIME_LIMIT=$((10)) # seconds
SEEDS=( 1 2 )
INSTANCES_FILE="small_benchmark_instances.txt" # file listing file paths to graphs
INSTANCE_NAMES_FILE="small_benchmark_instances_names.txt" # file listing filenames
BIN_DIR="../build/app"

export TASKS=60 # tasks in parallel
export GB_PER_TASK=200 # GB

# we name option that we toggle off (f)/on (n)
# we add the exact configuration to obtain optimal solutions
declare -a CONFIGS=(
	"${reduction_style}_rnt_exact"
	"${reduction_style}_drp"
	"${reduction_style}_drp_core_exact"
	"${reduction_style}_drp_core_chils"
	"${reduction_style}_rnt_chils"
	"${reduction_style}_rnt_htwis"
	"${reduction_style}_rnt_hils"
	"${reduction_style}_rnt_mmwis"
)


# for each config, define how a job is built
declare -a CONFIG_CREATORS=(
	"c_rnt_exact"
	"c_drp"
	"c_drp_core_exact"
	"c_drp_core_chils"
	"c_rnt_chils"
	"c_rnt_htwis"
	"c_rnt_hils"
	"c_rnt_mmwis"
)

############
# helpers
############
function c_solver_cli() {
	local instance=$1
	local config=$2
	local outdir=$3
	local seed=$4
	local time_limit=$5
	local solver=$6
	shift 6

	echo "./${BIN_DIR}/$solver" \
		"$@"  \
		"--time_limit=$time_limit --seed=$seed --reduction_style=$reduction_style $instance" \
	       	"> $outdir/$config/s${seed}-$(basename $instance).out 2>&1"
}

############
# different solvers
############
function c_rnt_exact() {
    c_solver_cli "$@" "redw2pack_rnt_exact"
}
function c_drp() {
    c_solver_cli  "$@" "redw2pack_drp" "--core_solver=none"
}
function c_drp_core_exact() {
    c_solver_cli "$@" "redw2pack_drp" "--core_solver=exact" "--sim_thres=0.8" "--increase_sim_thres=1.05" "--decrease_sim_thres=0.95"
}
function c_drp_core_chils() {
    c_solver_cli "$@" "redw2pack_drp" # default
}
function c_rnt_chils() {
    c_solver_cli "$@" "redw2pack_rnt_chils"
}
function c_rnt_htwis() {
    c_solver_cli "$@" "redw2pack_rnt_htwis"
}
function c_rnt_hils() {
    c_solver_cli "$@" "redw2pack_rnt_hils"
}
function c_rnt_mmwis() {
    c_solver_cli "$@" "redw2pack_rnt_mmwis"
}

export TIME_LIMIT
export SEEDS
export CONFIGS
export CONFIG_CREATORS
export INSTANCES_FILE
export INSTANCE_NAMES_FILE