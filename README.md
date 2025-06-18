# BreakID
A new symmetry detecting and breaking library. This is based on Jo Devriendt's [BreakID code](https://bitbucket.org/krr/breakid/src/master/). It has been re-licensed by the original author to be MIT and hence it's realeased as MIT here. All modifications by Mate Soos.

## Compiling
It is strongly recommended to not build, but to use the precompiled
binaries as in our [release](https://github.com/meelgroup/breakid/releases).
The second best thing to use is Nix. Simply [install
nix](https://nixos.org/download/) and then:
```shell
git clone https://github.com/meelgroup/breakid
cd breakid
nix shell
```

Then you will have `breakid` binary available and ready to use.

If this is somehow not what you want, you can also build it. See the [GitHub
Action](https://github.com/meelgroup/breakid/actions/workflows/build.yml) for the
specific set of steps, mostly:
```bash
git clone https://github.com/meelgroup/breakid
cd breakid
mkdir build && cd build
cmake ..
make
```

## Running BreakID
BreakID detects symmetries in your input CNF file and creates a new CNF file
that has your original CNF in it, along with some new variables and clauses
that help break most symmetries:

```bash
./breakid myfile.cnf symmetry-broken-output.cnf
c BreakID version [...]
c Detecting symmetry groups...
c Finished symmetry breaking. T: 3.78 s  T-out: N T-rem: 1.00
c Num generators: 42
[...]
c Constructing symmetry breaking formula...
[...]
c regular symmetry breaking clauses added: 3007
c row interchangeability breaking clauses added: 0
c total symmetry breaking clauses added: 3007
c auxiliary variables introduced: 988
```

The symmetry-broken CNF has been written to `symmetry-broken-output.cnf`. This
CNF has 3007 more clauses and 988 extra variables. The system took 3.78s to
find the generators for the symmetries, and it found a total of 42 generators.
In case you are interested in the generators, you can increase verbosity with
`--verb N`, and see the generators themselves.

If you run `./breakid` with the option `--help`, you will be given all the
different options your can pass.

## Library Use
Check out the `breakid-main.cpp` file for example usage and the `breakid.hpp`
header file for the API. The library should be fairly simple to use, but please do
ask away in case something is unclear.

# Example CNFs
To obtain the example CNFs:

```
git submodule update --init
cd examples
```

This folder contains highly symmetrical test cnfs. Particular attention goes to instances in `channel` and `counting`, which both exhibit row interchangeability due to high-level variable or value interchangeability.
