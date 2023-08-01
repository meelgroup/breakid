#!/bin/bash

set -e

rm -rf Make* cm* CM* lib* bliss breakid include src lib breakidConfig.cmake breakidTargets.cmake

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make -j6

