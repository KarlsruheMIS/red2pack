# red2pack

## Description
This project provides a branch-and–reduce and heuristic solver for the maximum 2-packing set problem.
They apply novel 2-packing set reductions exhaustively in a preprocessing step and transforms the kernel an equivalent MIS instance.

## Dependencies
- GCC 13.1.0 or newer
- cmake 3.16 or newer

## Setup (Linux/MacOS)
Run the following to setup git submodules.
```shell
git submodule --init update
```

## Build and use our algorithms (Linux/MacOS)
The implementation of our algorithms are located in `lib/algorithms`. 
We solve the MIS problem using the wighted branch-and-reduce and OnlineMIS solver of KaMIS. Please, make sure to run the `init.sh` script beforehand.

To build the executable, please run:
```shell
bash compile_withcmake.sh
```

The scripts installs two binaries in `./branch_and_reduce/deploy`.
They allow you to benchmark our algorithms:
- `red2pack_branch_and_reduce` using the weighted-branch-and-reduce solver from KaMIS
- `red2pack_heuristic` using OnlineMIS

Use `--help` for an overview of the options. Probably, the following script does all what you want:
```bash
RED2PACK_SOLVER=./deploy/red2pack_branch_and_reduce # or /deploy/red2pack_heuristic
RED2PACK_REDUCTION_STYLE=elaborated # or "core" or "none"
SEED=0
TIMELIMIT=300 # seconds
GRAPH=tests/graphs/lesmis.graph
LOG=$SEED-$TIMELIMIT-$RED2PACK_REDUCTION_STYLE-$(basename $RED2PACK_SOLVER)-$(basename ${GRAPH%.graph}).log
$RED2PACK_SOLVER $GRAPH --seed=$SEED --timelimit=$TIMELIMIT --reduction_style2=$RED2PACK_REDUCTION_STYLE --console_log=$LOG
```

## How to use our reduction pack `red2pack`
If you want to use only our reduction pack `red2pack`, e.g. you want to build your own maximum 2-packing-set solver,
then the following tutorial will help you.

1. Start a new CMake project,
2. Install red2pack as a recursive submodule
3. Start a new class for your solver that inherits from `lib/solver_scheme.h` and implement `solve_mis` that solves the MIS problem.
4. Add a main function that calls your solver (similar to `/app/bnr/red2pack_branch_and_reduce.cpp`).
5. Add the following in your CMakeLists.txt and adapt it to your setup  
```cmake
include_directories(path/to/red2pack)
add_subdirectory(path/to/red2pack)
add_executable(solver solver.cpp $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:libred2pack>)
```

## License (TODO)
The project is released under MIT. However, some files used for kernelization are released under the BSD 3-clause license. See the respective files for their license. If you publish results using our algorithms, please acknowledge our work by quoting one or more of the following papers:

For our reduction pack `red2pack` or one of our solvers `red2pack_branch_and_reduce` or `red2pack_heuristic` for the maximum 2-packing set problem,
please cite:
```text
@article{DBLP:journals/corr/abs-2308-15515,
  author       = {Jannick Borowitz and
                  Ernestine Gro{\ss}mann and
                  Christian Schulz and
                  Dominik Schweisgut},
  title        = {Finding Optimal 2-Packing Sets on Arbitrary Graphs at Scale},
  journal      = {CoRR},
  volume       = {abs/2308.15515},
  year         = {2023},
  url          = {https://doi.org/10.48550/arXiv.2308.15515},
  doi          = {10.48550/arXiv.2308.15515},
  eprinttype    = {arXiv},
  eprint       = {2308.15515},
  timestamp    = {Mon, 04 Sep 2023 15:29:24 +0200},
  biburl       = {https://dblp.org/rec/journals/corr/abs-2308-15515.bib},
  bibsource    = {dblp computer science bibliography, https://dblp.org}
}
```

If you use `red2pack_branch_and_reduce`, please also cite the following work because we solve the MIS problem using the weighted branch-and-reduce solver from KaMIS:
```text
@inproceedings{DBLP:conf/alenex/Lamm0SWZ19,
author    = {Sebastian Lamm and
Christian Schulz and
Darren Strash and
Robert Williger and
Huashuo Zhang},
title     = {Exactly Solving the Maximum Weight Independent Set Problem on Large Real-World Graphs},
booktitle = {Proceedings of the Twenty-First Workshop on Algorithm Engineering and Experiments, {ALENEX} 2019},
pages     = {144--158},
year      = {2019},
url       = {https://doi.org/10.1137/1.9781611975499.12},
doi       = {10.1137/1.9781611975499.12},
publisher = {{SIAM}},
year      = {2019}
}
```

If you use `red2pack_heuristic`, please also cite the following work because we solve the MIS problem using OnlineMIS from KaMIS:
```text
@inproceedings{DBLP:conf/wea/DahlumLS0SW16,
  author    = {Jakob Dahlum and
               Sebastian Lamm and
               Peter Sanders and
               Christian Schulz and
               Darren Strash and
               Renato F. Werneck},
  title     = {Accelerating Local Search for the Maximum Independent Set Problem},
  booktitle = {15th International Symposium on Experimental Algorithms {SEA}},
  pages     = {118--133},
  year      = {2016},
  series    = {Lecture Notes in Computer Science},
  volume    = {9685},
  publisher = {Springer},
  url       = {https://doi.org/10.1007/978-3-319-38851-9\_9}
}
```
