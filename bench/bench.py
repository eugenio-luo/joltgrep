#!/usr/bin/env python3

import time
import subprocess

WARMUP_SAMPLES = 3
EXPERIMENT_SAMPLES = 20
PATTERN = "queue" 
PATH = "../bulbOS/src"

def run(cmd):
    start = time.perf_counter_ns()
    ret = subprocess.run(cmd, 
                   capture_output=True, text=True)
    end = time.perf_counter_ns()

    return ret, end - start;

def print_result(ret, time):
    print("command: {0}, time: {1:.3f} ms".format(ret.args, time / 1000000))
    if (ret.returncode != 0):
        print(ret.stdout)
        print("Errors:\n{0}".format(ret.stderr));

def benchmark(cmd):
    for i in range(0, WARMUP_SAMPLES):
        run(cmd)

    total_time = 0
    for i in range(0, EXPERIMENT_SAMPLES):
        ret, time = run(cmd)
        total_time += time

    print_result(ret, total_time / EXPERIMENT_SAMPLES)

def main():
    cmds = [["build/bin/joltgrep", PATTERN, PATH],
            ["grep", "-r", PATTERN, PATH],
            ["rg", "-N", PATTERN, PATH]]

    for cmd in cmds:
        benchmark(cmd)

if __name__ == "__main__":
    main()