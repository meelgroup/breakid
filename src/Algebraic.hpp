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

class Specification;
class Breaker;

class Permutation : public std::enable_shared_from_this<Permutation> {
private:
  std::unordered_map<uint, uint> perm;
  std::vector<uint> cycleReprs; // smallest lit in each cycle
  uint maxCycleSize;
  size_t hash;

  

public:
  std::vector<uint> domain;
  std::vector<uint> posDomain;
  std::vector<uint> image;
  
  void addFromTo(uint from, uint to);
  void addCycle(std::vector<uint>& cyc);

  Permutation();
  Permutation(std::vector<std::pair<uint, uint> >& tuples);
  // Permutation constructed from swapping two rows.
  Permutation(std::vector<uint>& row1, std::vector<uint>& row2);

  ~Permutation() {
  };

  uint getImage(uint from);
  // return value is true iff the image is different from the original
  bool getImage(std::vector<uint>& orig, std::vector<uint>& img);
  void getCycle(uint lit, std::vector<uint>& orb);
  bool isInvolution();
  bool permutes(uint lit);
  uint supportSize();
  bool isIdentity();

  void print(std::ostream& out);

  bool formsMatrixWith(sptr<Permutation> other);
  std::pair<sptr<Permutation>, sptr<Permutation> > getLargest(sptr<Permutation> other);
  void getSharedLiterals(sptr<Permutation> other, std::vector<uint>& shared);
  std::vector<uint>& getCycleReprs();
  uint getMaxCycleSize();
  uint getNbCycles();

  bool equals(sptr<Permutation> other);
};

class Matrix {
private:
  std::vector<std::vector<uint>* > rows; // TODO: refactor this as 1 continuous vector
  std::unordered_map<uint, uint> rowco;
  std::unordered_map<uint, uint> colco;

public:
  Matrix();
  ~Matrix();
  void print(std::ostream& out);

  void add(std::vector<uint>* row);
  uint nbColumns();
  uint nbRows();
  void tryToAddNewRow(sptr<Permutation> p, uint rowIndex, Specification* theory);
  std::vector<uint>* getRow(uint rowindex);
  bool permutes(uint x);
  uint getLit(uint row, uint column);

  uint getRowNb(uint x);
  uint getColumnNb(uint x);

  sptr<Permutation> testMembership(const sptr<Permutation> p);
  sptr<Permutation> getProductWithRowsWap(const sptr<Permutation> p, uint r1, uint r2); // return p*swap(r1,r2)
};

class Group {
private:
  std::vector<sptr<Permutation> > permutations;
  std::vector<sptr<Matrix> > matrices;
  std::unordered_set<uint> support;

  void cleanPermutations(sptr<Matrix> matrix); // remove permutations implied by the matrix

public:
  // NOTE: if a group has a shared pointer to a theory, and a theory a shared pointer to a group, none of the memory pointed to by these pointers will ever be freed :(
  Specification* theory; // non-owning pointer

  Group() {
  };

  ~Group() {
  };

  void add(sptr<Permutation> p);
  void checkColumnInterchangeability(sptr<Matrix> m);

  void print(std::ostream& out);

  sptr<Matrix> getInitialMatrix();
  
  void addMatrices();
  void addMatrix(sptr<Matrix> m); // cnf-parameter, otherwise we have to store a pointer to the cnf here :(
  uint getNbMatrices();
  uint getNbRowSwaps();
  
  sptr<Matrix> getMatrix(uint idx);

  void getDisjointGenerators(std::vector<sptr<Group> >& subgroups);
  uint getSize();

  bool permutes(uint lit);
  uint getSupportSize();

  void getOrderAndAddBinaryClausesTo(Breaker& brkr, std::vector<uint>& out_order); // returns a vector containing a lit for literals relevant to construct sym breaking clauses
  void addBinaryClausesTo(Breaker& brkr, std::vector<uint>& out_order, const std::unordered_set<uint>& excludedLits);
  void addBreakingClausesTo(Breaker& brkr);

  void maximallyExtend(sptr<Matrix> matrix, uint indexOfFirstNewRow);
};
