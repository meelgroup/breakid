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

#include <vector>
#include <memory>
#include <string>
#include <set>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iostream>
#include <unordered_set>

#include "config.hpp"
#include "breakid/solvertypesmini.hpp"

using namespace BID;
using std::vector;
using std::set;
using std::endl;
using std::cout;
using std::cerr;
using std::string;
using std::shared_ptr;
using std::unordered_set;

class Permutation;
class Clause;
class OnlCNF;

template <class T>
void swapErase(vector<T>& vec, uint32_t index)
{
    vec[index] = vec.back();
    vec.pop_back();
}

inline size_t _getHash(const vector<BLit>& xs)
{
    size_t seed = xs.size();
    for (auto x : xs) {
        seed ^= x.toInt() + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

inline size_t _getHash(const BLit* lit, uint32_t sz)
{
    size_t seed = sz;
    for (uint32_t i = 0; i < sz; i++) {
        seed ^= lit[i].toInt() + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

class Clause
{
   private:
    size_t hashValue;

   public:
    vector<BLit> lits;

    Clause(const std::set<BLit>& inLits)
        : hashValue(0), lits(inLits.cbegin(), inLits.cend())
    {
    }

    Clause() : hashValue(0){};

    ~Clause(){};

    size_t getHashValue()
    {
        if (hashValue == 0) {
            hashValue = _getHash(lits);
            if (hashValue == 0) {
                // avoid recalculation of hash
                hashValue = 1;
            }
        }
        return hashValue;
    }

    void print(std::ostream& ostr)
    {
        for (auto lit : lits) {
            ostr << lit << " ";
        }
        ostr << "0\n";
    }
};

template <class T>
bool isDisjoint(std::unordered_set<T>& uset, vector<T>& vec)
{
    for (T x : vec) {
        if (uset.count(x) > 0) {
            return false;
        }
    }
    return true;
}

struct MyCl
{
    BLit* lits;
    uint32_t sz;
    uint32_t hashValue;
};

struct UVecHash {
    size_t operator()(const std::shared_ptr<vector<BLit> > first) const
    {
        return _getHash(*first);
    }

    size_t operator()(const std::shared_ptr<Clause> cl) const
    {
        return cl->getHashValue();
    }
};

struct MyClHash {
        size_t operator()(const MyCl& cl) const
    {
        return cl.hashValue;
    }
};

struct MyClEqual {
    bool operator()(
        const MyCl& first,
        const MyCl& second) const
    {
        if (first.sz != second.sz) {
            return false;
        }

        if (first.hashValue != second.hashValue) {
            return false;
        }

        for (unsigned int k = 0; k < first.sz; ++k) {
            if (first.lits[k] != second.lits[k]) {
                return false;
            }
        }
        return true;
    }
};

struct UvecEqual {
    bool equals(const vector<BLit>& first,
                const vector<BLit>& second) const
    {
        if (first.size() != second.size()) {
            return false;
        }
        for (unsigned int k = 0; k < first.size(); ++k) {
            if (first.at(k) != second.at(k)) {
                return false;
            }
        }
        return true;
    }

    bool operator()(const std::shared_ptr<vector<BLit> > first,
                    const std::shared_ptr<vector<BLit> > second) const
    {
        return equals(*first, *second);
    }

    bool operator()(const std::shared_ptr<Clause> first, const std::shared_ptr<Clause> second) const
    {
        return equals(first->lits, second->lits);
    }
};

class Breaker
{
public:
    Breaker(OnlCNF const* origTheo, Config* conf);
    ~Breaker(){};

    //Prints the current breaker
    void print(bool only_breakers);

    void addBinClause(BLit l1, BLit l2);
    void addSym(
        shared_ptr<Permutation> perm
        , vector<BLit>& order
        , bool limitExtraConstrs
    );
    vector<vector<BLit>> get_brk_cls();

    uint32_t getAuxiliaryNbVars();
    uint32_t getTotalNbVars();
    uint32_t getAddedNbClauses();
    uint32_t getTotalNbClauses();

    uint32_t getNbBinClauses();
    uint32_t getNbRowClauses();
    uint32_t getNbRegClauses();
    BLit getTseitinVar();

private:
    unordered_set<shared_ptr<Clause>, UVecHash, UvecEqual> clauses;
    OnlCNF const* originalTheory;
    uint32_t nbExtraVars = 0;
    uint32_t nbBinClauses = 0;
    uint32_t nbRowClauses = 0;
    uint32_t nbRegClauses = 0;

    void addBinary(BLit l1, BLit l2);
    void addTernary(BLit l1, BLit l2, BLit l3);
    void addQuaternary(BLit l1, BLit l2, BLit l3, BLit l4);
    void add(shared_ptr<Clause> cl);
    void addBreakID(shared_ptr<Permutation> perm, vector<BLit>& order,
             bool limitExtraConstrs);
    void addShatter(shared_ptr<Permutation> perm, vector<BLit>& order,
                    bool limitExtraConstrs);

    Config* conf;
};

#endif
