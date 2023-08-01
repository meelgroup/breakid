#!/bin/bash

set -e

rm -rf Make* cm* CM* lib* bliss breakid include src lib
cmake -DSTATICCOMPILE=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make -j6

