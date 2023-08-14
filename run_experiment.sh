#!/bin/bash
#
# This is a script to run all algorithms (including competitors)
# for a single graph instance.
#
# Usage: bash run_experiment.sh <path_to_graph_name> <use_gen2pack:0:1> <use_apx2p:0:1>
#

graph_path=${1#.graph}
graph=$(basename $graph_path)
use_gen2pack=$2
use_apx2p=$3
time_limit=$4 # seconds

out_dir="out_experiment"
mkdir -p $out_dir

seed=0
# 2pack
results="$graph"
log="${out_dir}/2pack_s${seed}_${graph}.log"
./branch_and_reduce/deploy/m2s_branch_and_reduce "$graph_path.graph" \
  "--seed=$seed" "--time_limit=$time_limit" \
  "--disable_deg_one" "--disable_deg_two" "--disable_twin" \
  "--disable_domination" "--disable_fast_domination" "--disable_clique" \
  >"$log" 2>&1

result="$results,$(python3 eval_experiment.py "2pack" "$log" $time_limit|awk -v OFS=, '{print $1,$2,$3,$4,$5}')"


# red2pack core
log="${out_dir}/red2pack_core_s${seed}_${graph}.log"
./branch_and_reduce/deploy/m2s_branch_and_reduce "${graph_path}.graph" \
  "--seed=$seed" "--time_limit=$time_limit" \
  "--reduction_style2=core" \
  >"$log" 2>&1

result="$result,$(python3 eval_experiment.py "red2pack" "$log" $time_limit|awk -v OFS=, '{print $1,$2,$3,$4,$5}')"

# red2pack elaborated
log="${out_dir}/red2pack_elaborated_s${seed}_${graph}.log"
./branch_and_reduce/deploy/m2s_branch_and_reduce "${graph_path}.graph" \
  "--seed=$seed" "--time_limit=$time_limit" \
  "--reduction_style2=elaborated" \
  >"$log" 2>&1

result="$result,$(python3 eval_experiment.py "red2pack" "$log" $time_limit|awk -v OFS=, '{print $1,$2,$3,$4,$5}')"

# competitor: gen2pack
if [ $use_gen2pack == "1" ]; then
    if [ ! -f ./competitor/Gene2Pack/wake.py ]; then
      echo "Competitor algorithm gen2pack not found! Skipping gen2pack!" >&2
      result="$result,-,-"
    else
      source competitor/Gene2Pack/venv/bin/activate
      log="$out_dir/gen2pack_${graph}.log"

      python3 competitor/Gene2Pack/wake.py "$graph_path.gml" "$out_dir" >"$log" 2>&1

      result="$result,$(python3 eval_experiment.py "gen2pack" "$out_dir/results_${graph}_wake.csv" $time_limit|awk -v OFS=, '{print $1,$2}')"
    fi
else
  result="$result,-,-"
fi

# competitor: Apx-2P+Im2P
if [ $use_apx2p == "1" ]; then
    if [ ! -f ./competitor/Approximate2Packing/deploy/apx-2p ]; then
      echo "Competitor algorithm Apx2P+Im2P not found! Skipping Apx2P+Im2P!" >&2
      result="$result,-,-"
    else
      log="$out_dir/apx2p_s${seed}_${graph}.log"

      ./competitor/Approximate2Packing/deploy/apx-2p "$graph_path.gml" >"$log" 2>&1

      result="$result,$(python3 eval_experiment.py "apx-2p" "$log" $time_limit|awk -v OFS=, '{print $1,$2}')"
    fi
else
  result="$result,-,-"
fi

echo $result