# Benchmark solvers for MW2PS

We provide some benchmark configurations and scripts to benchmark the solvers and reduce algorithms for the weighted problem.

To run all solvers for a fixed reduction style for a set of instances (graphs), you can use or adjust the configuration `benchmark_cfg_weighted_solvers.sh`.
Then you can call `./generate_benchmark.sh benchmark_cfg_weighted_solvers.sh`.
This creates the output directories and prints the jobs to a file.
You can run the jobs in parallel, e.g. with GNU parallel.
After running the benchmark you can parse the results (solution quality, time best, memory peak, remaining nodes, remaining edges, remaining links) from the log files.
This can be achieved with `./extract_benchmark.sh bencharm_cfg_weighted_solvers.sh <out_dir> <result_dir>`.

To benchmark the different reduction styles, you can use/adjust `benchmark_cfg_weighted_reduce.sh`.
