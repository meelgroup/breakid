#pragma once

#define sptr std::shared_ptr

#include <string>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <functional>
#include <cassert>
#include <time.h>
#include <algorithm>

typedef unsigned int uint;

// GLOBALS:
extern uint nVars;
extern std::vector<uint> fixedLits;
extern std::string inputSymFile;
extern time_t startTime;

// OPTIONS:
extern int symBreakingFormLength;
extern bool useBinaryClauses;
extern bool onlyPrintBreakers;
extern bool printGeneratorFile;
extern bool useMatrixDetection;
extern bool useShatterTranslation;
extern bool useFullTranslation;
extern uint verbosity;
extern int timeLim;

size_t _getHash(const std::vector<uint>& xs);
size_t _getHash(const std::vector<int>& xs);
int timeLeft();
bool timeLimitPassed();

inline bool sign(uint lit) {
  return lit & 1;
}

inline uint neg(uint lit) {
  return lit ^ 1;
}

inline uint encode(int lit) {
  return (lit > 0 ? 2 * (lit - 1) : 2 * (-lit - 1) + 1);
}

inline int decode(uint lit) {
  return (sign(lit) ? -(lit / 2 + 1) : lit / 2 + 1);
}

void gracefulError(std::string str);

class Clause {
private:
  size_t hashValue;

public:
  std::vector<uint> lits;

  Clause(const std::set<uint>& inLits) :
      hashValue(0), lits(inLits.cbegin(), inLits.cend()) {
  }

  Clause() :
      hashValue(0) {
  }
  ;

  ~Clause() {
  }
  ;

  size_t getHashValue() {
    if (hashValue == 0) {
      hashValue = _getHash(lits);
      if (hashValue == 0) {
        // avoid recalculation of hash
        hashValue = 1;
      }
    }
    return hashValue;
  }

  void print(std::ostream& ostr) {
    for (auto lit : lits) {
      ostr << decode(lit) << " ";
    }
    ostr << "0\n";
  }
  //Prints a clause c a rule falsevar <- (not c)
  //afterwards, falsevar (a new variable) will be added to the "false" constraints.
  void printAsRule(std::ostream& ostr, uint falsevar) {
    ostr << "1 " << falsevar << " ";
    std::set<uint> posBodyLits;
    std::set<uint> negBodyLits;
    for (auto lit : lits) {
      auto decoded = decode(lit);
      if (decoded > 0) {
        posBodyLits.insert(decoded);
      } else {
        negBodyLits.insert(-decoded);
      }
    }
    ostr << (posBodyLits.size() + negBodyLits.size()) << " " << negBodyLits.size() << " ";
    for (auto decodedlit : negBodyLits) {
      ostr << decodedlit << " ";
    }
    for (auto decodedlit : posBodyLits) {
      ostr << decodedlit << " ";
    }
    ostr << std::endl;
  }

};

class Rule {
private:
  size_t hashValue;

public:
  //Note: we use "constraints" as basic rules with an empty head. Thus, there is no nice correspondence between our representation of rules and the lparse-smodels format
  //Note: similarly, we use "disjunctive rules" as basic rules with multiple heads. (quite standard representation of those). Again: not nice  correspondence between our representation of rules and the lparse-smodels format
  //to respresent rules. As such, there is currently not yet a GOOD printing method for the rules.
  int ruleType;
  std::vector<uint> headLits;
  std::vector<uint> bodyLits;
  int bound; //Only relevant for rules of type 2 and 5
  std::vector<int> weights;

  Rule(int inRuleType, const std::vector<uint>& inHeads, const std::vector<uint>& bodies, int inBound, std::vector<int>& inWeights) :
      hashValue(0), ruleType(inRuleType), headLits(inHeads.cbegin(), inHeads.cend()), bodyLits(bodies.cbegin(), bodies.cend()), bound(inBound), weights(inWeights.cbegin(),
          inWeights.cend()) {
    std::sort(headLits.begin(), headLits.end());
    if(ruleType != 5 && ruleType != 6){
      std::sort(bodyLits.begin(), bodyLits.end());
    }
    assert(ruleType != 5 || ruleType != 6 || weights.size() == bodyLits.size());
  }

  Rule() :
      hashValue(0), ruleType(1), bound(0) {
  }
  ;

  ~Rule() {
  }
  ;

  size_t getHashValue() {
    if (hashValue == 0) {
      hashValue = ruleType + _getHash(headLits) + 2 * _getHash(bodyLits) + 3 * bound + 4 * _getHash(weights);
      if (hashValue == 0) {
        // avoid recalculation of hash
        hashValue = 1;
      }
    }
    return hashValue;
  }

  void print(std::ostream& /*ostr*/) {
    std::cerr << "Not implemented: printing rules\n";
    /*std::cerr << "WARNING: printing rules is NOT in the right order for lparse format\n";
     if (basicRule && headLits.size() == 0) {
     //A constraint
     assert(bodyLits.size() == 1);
     auto origLit = decode(bodyLits.front());
     if (origLit > 0) {
     ostr << "B-\n" << origLit;
     } else {
     ostr << "B+\n" << origLit;
     }
     } else {
     if (basicRule) {
     assert(headLits.size() == 1);
     ostr << "1 " << decode(headLits.front()) << " ";
     } else {
     ostr << "3 ";
     ostr << headLits.size() << " ";
     for (auto lit : headLits) {
     ostr << decode(lit) << " ";
     }
     }
     std::set<uint> posBodyLits;
     std::set<uint> negBodyLits;
     for (auto lit : bodyLits) {
     auto decoded = decode(lit);
     if (decoded > 0) {
     posBodyLits.insert(decoded);
     } else {
     negBodyLits.insert(-decoded);
     }
     }
     ostr << (posBodyLits.size() + negBodyLits.size()) << " " << negBodyLits.size() << " ";
     for (auto decodedlit : negBodyLits) {
     ostr << decodedlit << " ";
     }
     for (auto decodedlit : posBodyLits) {
     ostr << decodedlit << " ";
     }
     }
     ostr << std::endl;*/
  }
};

struct UVecHash {

  size_t operator()(const sptr<std::vector<uint> > first) const {
    return _getHash(*first);
  }

  size_t operator()(const sptr<Clause> cl) const {
    return cl->getHashValue();
  }

  size_t operator()(const sptr<Rule> r) const {
    return r->getHashValue();
  }
};

struct UvecEqual {

  bool equals(const std::vector<int>& first, const std::vector<int>& second) const {
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

  bool equals(const std::vector<uint>& first, const std::vector<uint>& second) const {
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

  bool operator()(const sptr<std::vector<uint> > first, const sptr<std::vector<uint> > second) const {
    return equals(*first, *second);
  }

  bool operator()(const sptr<Clause> first, const sptr<Clause > second) const {
    return equals(first->lits, second->lits);
  }

  bool operator()(const sptr<Rule> first, const sptr<Rule > second) const {
    return (first->ruleType == second->ruleType) && equals(first->headLits, second->headLits) && equals(first->bodyLits, second->bodyLits) && equals(first->weights, second->weights) && (first->bound == second->bound);
  }
};

template<class T>
bool isDisjoint(std::unordered_set<T>& uset, std::vector<T>& vec) {
  for (T x : vec) {
    if (uset.count(x) > 0) {
      return false;
    }
  }
  return true;
}

template<class T>
void swapErase(std::vector<T>& vec, uint index) {
  vec[index] = vec.back();
  vec.pop_back();
}
