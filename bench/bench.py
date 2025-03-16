#!/usr/bin/env python3

import time
import subprocess
import statistics

WARMUP_SAMPLES = 3
EXPERIMENT_SAMPLES = 3

def run(cmd):
    start = time.perf_counter_ns()
    ret = subprocess.run(cmd, 
                   capture_output=True, text=True)
    end = time.perf_counter_ns()

    return ret, end - start;

def print_result(cmd, pattern, samples, ret):
    cmd = cmd + pattern
    sample_mean = statistics.mean(samples)
    sample_std = statistics.stdev(samples, sample_mean)
    
    print("{0}{1}{2} => {3} matches, {4:.3f} ms ± {5:.3f} ms"
          .format("'", " ".join(cmd), "'", ret.decode("ascii")[:-1], 
                  sample_mean / 1000000, sample_std / 1000000))

def benchmark(cmd, pattern):
    cmd = cmd + pattern
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
            ["rg", "-N"],
            ["ggrep", "-r"],
            ["build/bin/joltgrep"]
           ]
    patterns = [
                ["queue", "../linux/drivers"],
                ["queu.", "../linux/drivers"],
                ["queu[ei]", "../linux/drivers"]
               ]
    #patterns = [["queu.", "../linux/drivers/scsi/scsi_transport_fc.c"]]
    results = []

    for pattern in patterns:
        for cmd in cmds:
            samples, ret = benchmark(cmd, pattern)
            results.append((cmd, pattern, samples, ret))

    print()
    for cmd, pattern, samples, ret in results:
        print_result(cmd, pattern, samples, ret)
    print()

if __name__ == "__main__":
    main()
