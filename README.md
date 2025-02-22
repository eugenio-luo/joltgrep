# joltgrep

Implementation of the grep command line tool in C++.

Objective: Be faster than GNU grep.

## Preliminary Data

The project is still a work in progress, there are still lot of cases to handle
and work to do. But there are some encouraging results, for example
searching for literals.

The table below shows the performance of different `grep` when searching for `PM_RESUME` 
in `linux/drivers`, using default settings (so without line counting). The experiment
was run on macOS Sequoia Apple M2 8GB RAM. 
| Program | Time (s) | Matches | Version |
| -------- | ------- | ------- |  ------- | 
| joltgrep | 0.631 ± 0.011 | 26 | 0.1 |
| ripgrep | 0.541 ± 0.065 | 26 | 14.1 |
| GNU grep | 1.332 ± 0.027 | 26 | 3.11 |
| BSD grep | 10.400 ± 0.067 | 26 | 2.6 |

## Build

1. install headers and libraries for `absl` and `re2` in `lib` 
2. run
```
./build.sh
```

