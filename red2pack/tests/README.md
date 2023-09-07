# Tests

We provide ctests for our maximum 2-packing set solvers.
They test for maximality and ensure a 2-packing set was found.
The set of instances is a sample of common 2-packing set instances.

| Graph                  | Source                                              |
|------------------------|-----------------------------------------------------|
| aGraphsErdos40-8.graph | https://github.com/trejoel/Gene2Pack/tree/amcs      |
| cac100.graph           | https://github.com/trejoel/Gene2Pack/tree/amcs      |
| cond-mat-2005.graph    | https://dimacs10.github.io/archive/clustering.shtml |
| lesmis.graph           | https://dimacs10.github.io/archive/clustering.shtml |
| Outerplanar500_1.graph | https://github.com/trejoel/Approximate2Packing      |


## Build and run tests
To build and run the tests, execute the following snippet:
```bash
bash compile_with_cmake.sh Debug True
cd build
ctest
```