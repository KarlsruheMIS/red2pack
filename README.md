# 2-Packing Set

## Description
This project provides a branch-and–reduce solver for the maximum 2-packing set problem.
The branch-and-reduce solvers applies exhaustively novel 2-packing set reductions and transforms the kernel to solve it 
as a equivalent MIS instance.
This MIS instance is solved using the weighted branch-and-reduce solver from [KaMIS](https://github.com/KarlsruheMIS/KaMIS).

## Dependencies
- GCC 13.1.0 or newer
- cmake 3.16 or newer
- Python3 (for setup and benchmark)

## Setup (Linux/MacOS)
Clone the repository and execute the following commands:
```shell
bash init.sh
```

## Build binaries (Linux/MacOS)
To build the executable run 
```shell
bash compile_withcmake.sh
```


## License (TODO)
The project is released under MIT. However, some files used for kernelization are released under the BSD 3-clause license. See the respective files for their license. If you publish results using our algorithms, please acknowledge our work by quoting one or more of the following papers:

```text
ToDO
```