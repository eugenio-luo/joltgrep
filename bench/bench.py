#!/usr/bin/env python3

import time
import subprocess
import statistics

WARMUP_SAMPLES = 3
EXPERIMENT_SAMPLES = 30
PATTERN = "queue" 
PATH = "../linux/drivers"

def run(cmd):
    start = time.perf_counter_ns()
    ret = subprocess.run(cmd, 
                   capture_output=True, text=True)
    end = time.perf_counter_ns()

    return ret, end - start;

def print_result(cmd, samples, ret):
    sample_mean = statistics.mean(samples)
    sample_std = statistics.stdev(samples, sample_mean)
    
    print("{0}{1}{2} => {3} matches, {4:.3f} ms ± {5:.3f} ms"
          .format("'", " ".join(cmd), "'", ret.decode("ascii")[:-1], 
                  sample_mean / 1000000, sample_std / 1000000))

def benchmark(cmd):
    for i in range(0, WARMUP_SAMPLES):
        print("'", end="")
        print(" ".join(cmd), end="")
        print("'", end="")
        print(": ", end="")
        print("warmup", i, end="\r")
        run(cmd)

    print("\033[K", end="\r")

    samples = []
    for i in range(0, EXPERIMENT_SAMPLES):
        print("'", end="")
        print(" ".join(cmd), end="")
        print("'", end="")
        print(": ", end="")
        print("run", i, end="\r")
        ret, time = run(cmd)
        if ret.returncode != 0:
            print(ret.stdout)
            print("Errors:\n{0}".format(ret.stderr));
        samples.append(time)

    ret = subprocess.check_output(" ".join(cmd) + "| wc -l", shell=True)

    print("'", end="")
    print(" ".join(cmd), end="")
    print("'", end="")
    print(": \033[0;32mSuccess!\033[0;0m")
    return samples, ret 

def main():
    cmds = [
            ["ggrep", "-r", PATTERN, PATH],
            ["build/bin/joltgrep", PATTERN, PATH],
            ["rg", "-N", PATTERN, PATH]
           ]

    results = []

    for cmd in cmds:
        samples, ret = benchmark(cmd)
        results.append((cmd, samples, ret))

    print()
    for cmd, samples, ret in results:
        print_result(cmd, samples, ret)
    print()

if __name__ == "__main__":
    main()
