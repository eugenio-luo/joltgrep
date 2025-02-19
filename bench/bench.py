#!/usr/bin/env python3

import time
import subprocess
import statistics

WARMUP_SAMPLES = 3
EXPERIMENT_SAMPLES = 20
PATTERN = "queue" 
PATH = "../linux/arch"

def run(cmd):
    start = time.perf_counter_ns()
    ret = subprocess.run(cmd, 
                   capture_output=True, text=True)
    end = time.perf_counter_ns()

    return ret, end - start;

def print_result(cmd, samples):
    sample_mean = statistics.mean(samples)
    sample_std = statistics.stdev(samples, sample_mean)
    
    print("command: {0}, time: {1:.3f} ms ± {2:.3f} ms".format(cmd, sample_mean / 1000000, sample_std / 1000000))

def benchmark(cmd):
    print(cmd)
    for i in range(0, WARMUP_SAMPLES):
        print("warmup", i)
        run(cmd)

    samples = []
    for i in range(0, EXPERIMENT_SAMPLES):
        print("run", i)
        ret, time = run(cmd)
        if ret.returncode != 0:
            print(ret.stdout)
            print("Errors:\n{0}".format(ret.stderr));
        samples.append(time)

    return samples 

def main():
    cmds = [["build/bin/joltgrep", PATTERN, PATH],
            ["ggrep", "-r", PATTERN, PATH],
            ["rg", "-N", PATTERN, PATH]]

    results = []

    for cmd in cmds:
        results.append((cmd, benchmark(cmd)))

    for cmd, samples in results:
        print_result(cmd, samples)

if __name__ == "__main__":
    main()
