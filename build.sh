#!/bin/bash

cd build
cmake .. -DCMAKE_PREFIX_PATH=lib
cmake --build .
