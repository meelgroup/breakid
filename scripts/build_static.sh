#!/bin/bash

set -e

rm -rf Make* cm* CM* lib* bliss breakid include src lib
cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make -j$(nproc)

