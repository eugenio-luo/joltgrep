#!/bin/bash

rm -r build
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=lib
cmake --build .
