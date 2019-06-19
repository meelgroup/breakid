/******************************************
Copyright (c) 2019 Jo Devriendt - KU Leuven

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
***********************************************/

#ifndef THEORY_H
#define THEORY_H

#include "config.hpp"
#include "Breaking.hpp"
#include "breakid/solvertypesmini.hpp"
#include <unordered_set>

class Graph;
class Permutation;
class Group;
class Matrix;
class Breaker;

class OnlCNF
{
public:
    OnlCNF(Config* conf);
    ~OnlCNF();

    void end_dynamic_cnf();
    void add_clause(BID::BLit* lits, uint32_t size);
    void set_new_group();
    void set_old_group(Group* grp);
    const Graph* getGraph() const;
    Group* group = NULL;
    Graph* graph = NULL;

    void print(std::ostream& out) const;
    uint32_t getSize() const;
    void setSubTheory(Group* subgroup);
    bool isSymmetry(Permutation& prm);
    friend class Breaker;

private:
    Config* conf;
    uint32_t num_cls;
    vector<uint32_t> cl_sizes;
    vector<BLit> cl_lits;

    //ONLY filled when isSymmetry is called
    std::unordered_set<MyCl, MyClHash, MyClEqual> clauses;
};

#endif
