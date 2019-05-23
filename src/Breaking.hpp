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

#ifndef BREAKING_H
#define BREAKING_H

#include "global.hpp"

class Permutation;
class Specification;

class Breaker
{
public:
    Breaker(sptr<Specification> origTheo, Config* conf);
    ~Breaker(){};

    //Prints the current breaker. Gets the original file as input to recover information lost in the process
    void print(string& origfile);

    void addBinClause(uint32_t l1, uint32_t l2);
    void addRegSym(sptr<Permutation> perm, vector<uint32_t>& order);
    void addRowSym(sptr<Permutation> perm, vector<uint32_t>& order);

    uint32_t getAuxiliaryNbVars();
    uint32_t getTotalNbVars();
    uint32_t getAddedNbClauses();
    uint32_t getTotalNbClauses();

    uint32_t getNbBinClauses();
    uint32_t getNbRowClauses();
    uint32_t getNbRegClauses();
    uint32_t getTseitinVar();

private:
    std::unordered_set<sptr<Clause>, UVecHash, UvecEqual> clauses;
    sptr<Specification> originalTheory;
    uint32_t nbExtraVars = 0;
    uint32_t nbBinClauses = 0;
    uint32_t nbRowClauses = 0;
    uint32_t nbRegClauses = 0;

    void addBinary(uint32_t l1, uint32_t l2);
    void addTernary(uint32_t l1, uint32_t l2, uint32_t l3);
    void addQuaternary(uint32_t l1, uint32_t l2, uint32_t l3, uint32_t l4);
    void add(sptr<Clause> cl);
    void add(sptr<Permutation> perm, vector<uint32_t>& order,
             bool limitExtraConstrs);
    void addShatter(sptr<Permutation> perm, vector<uint32_t>& order,
                    bool limitExtraConstrs);

    Config* conf;
};

#endif
