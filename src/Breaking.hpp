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

inline size_t _getHash(const vector<uint32_t>& xs)
{
    size_t seed = xs.size();
    for (auto x : xs) {
        seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

inline size_t _getHash(const vector<int>& xs)
{
    size_t seed = xs.size();
    for (auto x : xs) {
        seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

inline bool sign(uint32_t lit)
{
    return lit & 1;
}

inline uint32_t neg(uint32_t lit)
{
    return lit ^ 1;
}

inline uint32_t encode(int lit)
{
    return (lit > 0 ? 2 * (lit - 1) : 2 * (-lit - 1) + 1);
}

inline int lit_to_weird(BID::BLit l)
{
    int t = l.var()+1;
    if (l.sign()) {
        t *= -1;
    }
    return t;
}

inline int decode(uint32_t lit)
{
    return (sign(lit) ? -(lit / 2 + 1) : lit / 2 + 1);
}

class Clause
{
   private:
    size_t hashValue;

   public:
    vector<uint32_t> lits;

    Clause(const std::set<uint32_t>& inLits)
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
            ostr << decode(lit) << " ";
        }
        ostr << "0\n";
    }
    //Prints a clause c a rule falsevar <- (not c)
    //afterwards, falsevar (a new variable) will be added to the "false" constraints.
    void printAsRule(std::ostream& ostr, uint32_t falsevar)
    {
        ostr << "1 " << falsevar << " ";
        std::set<uint32_t> posBodyLits;
        std::set<uint32_t> negBodyLits;
        for (auto lit : lits) {
            auto decoded = decode(lit);
            if (decoded > 0) {
                posBodyLits.insert(decoded);
            } else {
                negBodyLits.insert(-decoded);
            }
        }
        ostr << (posBodyLits.size() + negBodyLits.size()) << " "
             << negBodyLits.size() << " ";
        for (auto decodedlit : negBodyLits) {
            ostr << decodedlit << " ";
        }
        for (auto decodedlit : posBodyLits) {
            ostr << decodedlit << " ";
        }
        ostr << endl;
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
    vector<uint32_t> headLits;
    vector<uint32_t> bodyLits;
    int bound; //Only relevant for rules of type 2 and 5
    vector<int> weights;

    Rule(int inRuleType, const vector<uint32_t>& inHeads,
         const vector<uint32_t>& bodies, int inBound,
         vector<int>& inWeights)
        : hashValue(0),
          ruleType(inRuleType),
          headLits(inHeads.cbegin(), inHeads.cend()),
          bodyLits(bodies.cbegin(), bodies.cend()),
          bound(inBound),
          weights(inWeights.cbegin(), inWeights.cend())
    {
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
    size_t operator()(const std::shared_ptr<vector<uint32_t> > first) const
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

    bool equals(const vector<uint32_t>& first,
                const vector<uint32_t>& second) const
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

    bool operator()(const std::shared_ptr<vector<uint32_t> > first,
                    const std::shared_ptr<vector<uint32_t> > second) const
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

    void addBinClause(uint32_t l1, uint32_t l2);
    void addSym(
        shared_ptr<Permutation> perm
        , vector<uint32_t>& order
        , bool limitExtraConstrs
    );
    vector<vector<BID::BLit>> get_brk_cls();

    uint32_t getAuxiliaryNbVars();
    uint32_t getTotalNbVars();
    uint32_t getAddedNbClauses();
    uint32_t getTotalNbClauses();

    uint32_t getNbBinClauses();
    uint32_t getNbRowClauses();
    uint32_t getNbRegClauses();
    uint32_t getTseitinVar();

private:
    unordered_set<shared_ptr<Clause>, UVecHash, UvecEqual> clauses;
    Specification const* originalTheory;
    uint32_t nbExtraVars = 0;
    uint32_t nbBinClauses = 0;
    uint32_t nbRowClauses = 0;
    uint32_t nbRegClauses = 0;

    void addBinary(uint32_t l1, uint32_t l2);
    void addTernary(uint32_t l1, uint32_t l2, uint32_t l3);
    void addQuaternary(uint32_t l1, uint32_t l2, uint32_t l3, uint32_t l4);
    void add(shared_ptr<Clause> cl);
    void add(shared_ptr<Permutation> perm, vector<uint32_t>& order,
             bool limitExtraConstrs);
    void addShatter(shared_ptr<Permutation> perm, vector<uint32_t>& order,
                    bool limitExtraConstrs);

    Config* conf;
};

#endif
