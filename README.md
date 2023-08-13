# 2-Packing Set

## Description
This project provides a branch-and–reduce solver for the maximum 2-packing set problem.
The branch-and-reduce solver applies exhaustively novel 2-packing set reductions and transforms the kernel to solve it 
as an equivalent MIS instance.
This MIS instance is solved using the weighted branch-and-reduce solver from [KaMIS](https://github.com/KarlsruheMIS/KaMIS).

## Dependencies
- GCC 13.1.0 or newer
- cmake 3.16 or newer
- Python3 (for setup and benchmark)

## Setup (Linux/MacOS)
In following, the script downloads external benchmark sets from GitHub,
and translates them from the GML format to the METIS graph format.
All graphs are moved to `./graphs/`.
Execute the following commands:
```shell
bash init.sh
```

## Build and use our algorithms (Linux/MacOS)
The implementation of our algorithms `red2pack core/elaborated` and `2pack` are located in `./branch_and_reduce`.
We solve the MIS problem using the wighted branch-and-reduce solver of KaMIS. Please, make sure run the `init.sh` script beforehand.


To build the executable run:
```shell
cd branch_and_reduce && bash compile_withcmake.sh
```

The scripts installs four binaries in `./branch_and_reduce/deploy`:
- `m2s_branch_and_reduce` provides our the implementations for `red2pack core/elaborated` and `2pack`
- `check_connected` to check whether a graph in connected. Is is used for the competitor algorithm `gen2pack`
- `check_graph` to check whether a M2PS-to-MIS condensed graph (with or without reductions) is small enough for the weighted branch-and-reduce solver from KaMIS (32 bit limit for Node/Edge representation)
- `graph_to_gml` translates an undirected unweighted graph from the METIS format to the GML format

Use `--help` for an overview of the options or take a look in our example experiment `run_expriment.sh`.

## Experiment
The following experiment works out of the box for a few example instances.
Just clone this repository and run `bash example_experiment.sh`.
The solution size of the 2pack and the run time is written to `results.csv`.
For more details of the execution you can take a look into the output of the algorithms
in `./out_experiment`.
```shell
# init
INIT=1
if [ $INIT == 1 ]; then
    bash init.sh || exit
    cd branch_and_reduce ||exit
    bash compile_withcmake.sh || exit
    cd ..
    cd competitor/Gene2Pack ||exit
    bash init.sh || exit
fi

# example_experiment.sh
results="results.csv"
time_limit=10 # seconds
rm $results
echo "graph,2pack,,,,,red2pack core,,,,,red2pack elaborated,,,,,gen2pack,,Apx-2p+Imp2p," >> $results
echo ",S,t,t_p,n,m,S,t,t_p,n,m,S,t,t_p,n,m,S,t,S,t" >> $results # solution + time found

# Usage: bash run_experiment.sh <path_to_graph_filename> <use_genpack:1:0> <use_apx2p:1:0> <time_limit_in_sec>

# Erdos-Renyi graph from https://github.com/trejoel/Gene2Pack/tree/amcs
bash run_experiment.sh graphs/example/aGraphErdos40-1 1 0 $time_limit >> $results
echo "Finished aGraphErdos40-1"
# Cactus graph from https://github.com/trejoel/Gene2Pack/tree/amcs
bash run_experiment.sh graphs/example/cac100 1 0 $time_limit>> $results
echo "Finished cac100"
# Outer planar graph from https://github.com/trejoel/Approximate2Packing
bash run_experiment.sh graphs/example/Outerplanar500_1 0 1 $time_limit >> $results
echo "Finished Outerplanar500_1"
# Social graph from https://dimacs10.github.io/archive/clustering.shtml
bash run_experiment.sh graphs/example/lesmis 0 0 $time_limit >> $results
echo "Finished lesmis"
bash run_experiment.sh graphs/example/cond-mat-2005 0 0 $time_limit >> $results
echo "Finished cond-mat-2005"
```

### Experiments with other instances
We provide a script to run all algorithms (including competitor algorithms) for a single graph.
Please note, that the competitor `gen2pack` works only for connected graphs
and `Apx-2p + Imp2p` require outer planar graphs as input.
If you want to use a competitor, make sure an equivalent `graph.gml` is located in the same directory where `path_to_graph_filename.graph` is stored.
To translate a `.graph` (METIS format) to GML feel free to use our re-formatter tool `./branch_and_reduce/deploy/graph_to_gml`. 
```shell
bash run_experiment.sh <path_to_graph_filename> <use_genpack:1:0> <use_apx2p:1:0> <time_limit_in_sec>
```


## License (TODO)
The project is released under MIT. However, some files used for kernelization are released under the BSD 3-clause license. See the respective files for their license. If you publish results using our algorithms, please acknowledge our work by quoting one or more of the following papers:

```text
ToDO
```
