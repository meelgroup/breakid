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
class Rule;
class Specification;

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

class Rule
{
   private:
    size_t hashValue;

   public:
    //Note: we use "constraints" as basic rules with an empty head. Thus, there is no nice correspondence between our representation of rules and the lparse-smodels format
    //Note: similarly, we use "disjunctive rules" as basic rules with multiple heads. (quite standard representation of those). Again: not nice  correspondence between our representation of rules and the lparse-smodels format
    //to respresent rules. As such, there is currently not yet a GOOD printing method for the rules.
    int ruleType;
    vector<BLit> headLits;
    vector<BLit> bodyLits;
    int bound; //Only relevant for rules of type 2 and 5
    vector<BLit> weights;

    Rule(int inRuleType, const vector<uint32_t>& inHeads,
         const vector<uint32_t>& bodies, int inBound,
         vector<BLit>& inWeights) :

    hashValue(0),
    ruleType(inRuleType),
    bound(inBound)
    {
        for(const auto& x: inWeights) {
            weights.push_back(x);
        }
        for(const auto& x: inHeads) {
            headLits.push_back(BLit::toBLit(x));
        }

        for(const auto& x: bodies) {
            bodyLits.push_back(BLit::toBLit(x));
        }


        std::sort(headLits.begin(), headLits.end());
        if (ruleType != 5 && ruleType != 6) {
            std::sort(bodyLits.begin(), bodyLits.end());
        }
        assert(ruleType != 5 || ruleType != 6 ||
               weights.size() == bodyLits.size());
    }

    Rule() : hashValue(0), ruleType(1), bound(0){};

    ~Rule(){};

    size_t getHashValue()
    {
        if (hashValue == 0) {
            hashValue = ruleType + _getHash(headLits) + 2 * _getHash(bodyLits) +
                        3 * bound + 4 * _getHash(weights);
            if (hashValue == 0) {
                // avoid recalculation of hash
                hashValue = 1;
            }
        }
        return hashValue;
    }

    void print(std::ostream& /*ostr*/)
    {
        std::cout << "ERROR: Not implemented: printing rules\n";
        assert(false);
        exit(-1);
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

struct UVecHash {
    size_t operator()(const std::shared_ptr<vector<BLit> > first) const
    {
        return _getHash(*first);
    }

    size_t operator()(const std::shared_ptr<Clause> cl) const
    {
        return cl->getHashValue();
    }

    size_t operator()(const std::shared_ptr<Rule> r) const
    {
        return r->getHashValue();
    }
};

struct UvecEqual {
    bool equals(const vector<int>& first,
                const vector<int>& second) const
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

    bool operator()(const std::shared_ptr<Rule> first, const std::shared_ptr<Rule> second) const
    {
        return (first->ruleType == second->ruleType) &&
               equals(first->headLits, second->headLits) &&
               equals(first->bodyLits, second->bodyLits) &&
               equals(first->weights, second->weights) &&
               (first->bound == second->bound);
    }
};

class Breaker
{
public:
    Breaker(Specification const* origTheo, Config* conf);
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
    Specification const* originalTheory;
    uint32_t nbExtraVars = 0;
    uint32_t nbBinClauses = 0;
    uint32_t nbRowClauses = 0;
    uint32_t nbRegClauses = 0;

    void addBinary(BLit l1, BLit l2);
    void addTernary(BLit l1, BLit l2, BLit l3);
    void addQuaternary(BLit l1, BLit l2, BLit l3, BLit l4);
    void add(shared_ptr<Clause> cl);
    void add(shared_ptr<Permutation> perm, vector<BLit>& order,
             bool limitExtraConstrs);
    void addShatter(shared_ptr<Permutation> perm, vector<BLit>& order,
                    bool limitExtraConstrs);

    Config* conf;
};

#endif
