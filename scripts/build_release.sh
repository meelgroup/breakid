#!/bin/bash

set -e

rm -rf cm* CM* lib* bliss breakid include src lib
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make -j6 VERBOSE=1

