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

#pragma once

#include "global.hpp"

class Permutation;
class Specification;

class Breaker
{
   private:
    std::unordered_set<sptr<Clause>, UVecHash, UvecEqual> clauses;
    sptr<Specification> originalTheory;
    uint nbExtraVars = 0;
    uint nbBinClauses = 0;
    uint nbRowClauses = 0;
    uint nbRegClauses = 0;

    void addBinary(uint l1, uint l2);
    void addTernary(uint l1, uint l2, uint l3);
    void addQuaternary(uint l1, uint l2, uint l3, uint l4);
    void add(sptr<Clause> cl);
    void add(sptr<Permutation> perm, std::vector<uint>& order,
             bool limitExtraConstrs);
    void addShatter(sptr<Permutation> perm, std::vector<uint>& order,
                    bool limitExtraConstrs);

   public:
    Breaker(sptr<Specification> origTheo);

    ~Breaker(){};

    //Prints the current breaker. Gets the original file as input to recover information lost in the process
    void print(std::string& origfile);

    void addBinClause(uint l1, uint l2);
    void addRegSym(sptr<Permutation> perm, std::vector<uint>& order);
    void addRowSym(sptr<Permutation> perm, std::vector<uint>& order);

    uint getAuxiliaryNbVars();
    uint getTotalNbVars();
    uint getAddedNbClauses();
    uint getTotalNbClauses();

    uint getNbBinClauses();
    uint getNbRowClauses();
    uint getNbRegClauses();

    uint getTseitinVar();
};
