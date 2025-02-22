# joltgrep

Implementation of the grep command line tool in C++.

Objective: Be faster than GNU grep.

## Build

1. install headers and libraries for `absl` and `re2` in `lib` 
2. run
```
./build.sh
```

## Preliminary Data

The project is still a work in progress, there are still lot of cases to handle
and work to do. But there are some encouraging results, for example
searching for literals.

The table below shows the performance of different `grep` when searching for `PM_RESUME` 
in `linux/drivers`, using default settings (so without line counting). The experiment
was run on macOS Sequoia Apple M2 8GB RAM. 
| Program | Time (ms) | Matches | Version |
| -------- | ------- | ------- |  ------- | 
| joltgrep | 630.93 ± 11.18 | 26 | 0.1 |
| ripgrep | 541.10 ± 64.84 | 26 | 14.1 |
| GNU grep | 1331.93 ± 26.90 | 26 | 3.11 |
| BSD grep | 10399.96 ± 67.04 | 26 | 2.6 |
