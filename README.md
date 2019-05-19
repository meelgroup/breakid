BreakID 2.3
========
Welcome to the repository of the second version of BreakID, a new symmetry detecting and breaking preprocessor for SAT solvers. A summary of BreakID's symmetry breaking techniques can be found in the file "BreakIDGlucose2.pdf", the solver description provided to the SATRACE '15 competition. The submodule "paper" contains a more detailed description of BreakID's symmetry breaking techniques (to obtain it, run `git submodule init; git submodule update --recursive --init`). The folder "experiments" contains results of experiments run in February 2016. The folder "test_cnfs" contains highly symmetrical test cnfs. Particular attention goes to instances in `channel` and `counting`, which both exhibit row interchangeability due to high-level variable or value interchangeability.

----------
Download
-------------
A statically compiled 64 bit Unix binary is provided in the "Downloads" section.

----------
Compile
-----------
Simply run "make" in the "src/" folder. The executable "BreakID" will be compiled into that folder. To clean the build, run "make clean" in "src/".

----------
Run
-----
Run
```brainfuck
./BreakID
```
or
```brainfuck
./BreakID -h
```
to get information on how to use BreakID and what its options are.

For example
```brainfuck
./BreakID ../test_cnfs/pigeonhole/hole002.cnf -v 0 -t 300 -s 100
```
uses the same arguments as in the SATRACE '15 competition.

As far as output is concerned, all verbosity output is sent to stderr. stdout will contain a Dimacs CNF file, with as first three lines
```brainfuck
c number of breaking clauses added: <total number of symmetry breaking clauses added to the cnf>
c max original variable: <highest variable in input cnf>
p cnf <new number of variables> <new number of clauses>
```
followed by a set of clauses equivalent to the original CNF theory augmented with symmetry breaking clauses.

----------
ASP support
----------

Starting from version 2.2, BreakID supports ASP files in the intermediate lparse/smodels or gringo/clasp format. 
A typical workflow for ASP solving using BreakID is:


```brainfuck
gringo <encoding> <instance> | ./BreakID - -asp  -v 0 -t 300 -s 100 | clasp
```

where - tells BreakID to read from stdin, and -v 0 -t 300 -s 100 is the configuration used in various competitions. 

----------
Changelog
-----
1.0: see [bitbucket.org/krr/symbreaker](https://bitbucket.org/krr/symbreaker)    
2.0: rewrite of BreakID    
2.1: streamlined command-line usage and lowered memory overhead    
2.2: Added support for ASP files
2.3: Fixed bug and added extra command line options
