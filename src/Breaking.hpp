/******************************************
Copyright (c) 2019 JO DEVRIENDT - KU LEUVEN
***********************************************/

#pragma once

#include "global.hpp"

class Permutation;
class Specification;

class Breaker {
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
  void add(sptr<Permutation> perm, std::vector<uint>& order, bool limitExtraConstrs);
  void addShatter(sptr<Permutation> perm, std::vector<uint>& order, bool limitExtraConstrs);

public:
  Breaker(sptr<Specification> origTheo);

  ~Breaker() {
  };

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
