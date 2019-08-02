BreakID 3.0
========
A new symmetry detecting and breaking library. This is based on Jo Devriendt's [BreakID code](https://bitbucket.org/krr/breakid/src/master/). It has been re-licensed by the original author to be MIT and hence it's realeased as MIT here. All modifications by Mate Soos.


Compile
-----

```
mkdir build
cd build
cmake ..
```

Library Use
-----
Please see the `test.cpp` file for usage and the `breakid.hpp` header file for the allowed set of calls. The library should be fairly obvious how to use.


Test Binary
-----
```brainfuck
./mytest myfile.cnf 8000
```

This runs the library on the CNF you supply and output the generators, etc.

Example CNFs
-----
To obtain:

```
git submodule update --init
cd examples
```

This folder contains highly symmetrical test cnfs. Particular attention goes to instances in `channel` and `counting`, which both exhibit row interchangeability due to high-level variable or value interchangeability.
