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

#ifndef GLOBAL_H
#define GLOBAL_H

#define sptr std::shared_ptr

#include <time.h>
#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <limits>

using std::cout;
using std::endl;
using std::vector;
using std::string;

// GLOBALS:
struct Config {
    uint32_t nVars = 0;
    vector<uint32_t> fixedLits;
    string inputSymFile;
    time_t startTime;

    // OPTIONS
    bool useMatrixDetection = true;
    bool useBinaryClauses = true;
    bool printGeneratorFile = false;
    bool useShatterTranslation = false;
    bool useFullTranslation = false;
    int symBreakingFormLength = 50;
    bool onlyPrintBreakers = false;
    uint32_t verbosity = 1;
    int64_t timeLim = std::numeric_limits<int64_t>::max();
};

size_t _getHash(const vector<uint32_t>& xs);
size_t _getHash(const vector<int>& xs);

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

inline int decode(uint32_t lit)
{
    return (sign(lit) ? -(lit / 2 + 1) : lit / 2 + 1);
}

void gracefulError(string str);

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
        std::cerr << "Not implemented: printing rules\n";
    }
};

struct UVecHash {
    size_t operator()(const sptr<vector<uint32_t> > first) const
    {
        return _getHash(*first);
    }

    size_t operator()(const sptr<Clause> cl) const
    {
        return cl->getHashValue();
    }

    size_t operator()(const sptr<Rule> r) const
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

    bool operator()(const sptr<vector<uint32_t> > first,
                    const sptr<vector<uint32_t> > second) const
    {
        return equals(*first, *second);
    }

    bool operator()(const sptr<Clause> first, const sptr<Clause> second) const
    {
        return equals(first->lits, second->lits);
    }

    bool operator()(const sptr<Rule> first, const sptr<Rule> second) const
    {
        return (first->ruleType == second->ruleType) &&
               equals(first->headLits, second->headLits) &&
               equals(first->bodyLits, second->bodyLits) &&
               equals(first->weights, second->weights) &&
               (first->bound == second->bound);
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

template <class T>
void swapErase(vector<T>& vec, uint32_t index)
{
    vec[index] = vec.back();
    vec.pop_back();
}

#endif
