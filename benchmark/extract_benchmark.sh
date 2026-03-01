#!/bin/bash

path=$(pwd)
benchmarkcfg=$1
source $benchmarkcfg
shift 1

exp_dir=$1
res_dir=$2
mkdir -p $res_dir
instances_f=$3

timeout_eps=5 # sec
relaxed_timelimit=$((timeout_eps + $TIME_LIMIT))


for config in "${CONFIGS[@]}"; do

    rm "$res_dir/$config.csv"


    while read instance; do

        for seed in "${SEEDS[@]}"; do

            out_f="$exp_dir/$config/s$seed-$instance.graph.out"
            if [ -f "$out_f" ]; then

                awk '/Size:/{sol=$2} /Time best:/{time=$3} /malloc_count/{gsub(",","",$7);mem=$7} /\|-Kernel Nodes/{knodes=$3} /\|-Kernel Edges/{kedges=$3} /\|-Kernel Links/{klinks=$3} END {print sol,time,mem,knodes,kedges,klinks}' $outf >> "$res_dir/$config.csv"

            else

                echo "$out_f does not exist!"

            fi

        done

    done <$instances_f


done
