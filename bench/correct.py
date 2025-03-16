#!/usr/bin/env python3

import subprocess

def check_output(cmds, arg):
    for cmd in cmds:
        cmd_str = " ".join(cmd) + " " + " ".join(arg)
        ret = subprocess.check_output( cmd_str + "| wc -l", shell=True)
        lines = ret.decode("ascii")[:-1]
        print("'{0}' => {1} matches"
          .format(cmd_str, ret.decode("ascii")[:-1])) 

    print()

def main():
    cmds = [ ["build/bin/joltgrep"],
             ["ggrep", "-r"] ] 
    
    args = [ ["queue", "../bulbOS/src"],
             ["queue", "../linux/drivers"],   
             ["PM_RESUME", "../linux/drivers"],
             ["queue", "../linux"],
             ["PM_RESUME", "../linux"] ] 

    for arg in args:
        check_output(cmds, arg)

if __name__ == "__main__":
    main()
