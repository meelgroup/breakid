
#include "global.hpp"

#include "Algebraic.hpp"
#include "Theory.hpp"
#include "Breaking.hpp"
#include "Graph.hpp"

#include <stdio.h>
#include <limits.h>

using namespace std;

void Permutation::addFromTo(uint from, uint to) {
  if (from != to && from < 2 * nVars && to < 2 * nVars) { // only ID's < 2*nVars represent literals
    perm[from] = to;
    domain.push_back(from);
    image.push_back(to);
    if (!sign(from)) {
      posDomain.push_back(from);
    }
  }
}

void Permutation::addCycle(std::vector<uint>& cyc){
  uint n = cyc.size();
  for(uint i=0; i<n; ++i){
    addFromTo(cyc.at(i),cyc.at((i+1)%n));
  }
}

Permutation::Permutation(){
  hash=0;
  maxCycleSize = 1;
}

Permutation::Permutation(std::vector<std::pair<uint, uint> >& tuples) {
  for (auto tup : tuples) {
    addFromTo(tup.first, tup.second);
  }
  hash = 0;
  maxCycleSize = 1;
}

Permutation::Permutation(std::vector<uint>& row1, std::vector<uint>& row2) {
  for (uint i = 0; i < row1.size() && i < row2.size(); ++i) {
    uint from = row1[i];
    uint to = row2[i];
    addFromTo(from, to);
    addFromTo(to, from);
  }
  hash = 0;
  maxCycleSize = 1;
}

uint Permutation::getImage(uint from) {
  auto it = perm.find(from);
  if (it != perm.end()) {
    return it->second;
  } else {
    return from;
  }
}

bool Permutation::getImage(std::vector<uint>& orig, std::vector<uint>& img) {
  img.clear();
  img.reserve(orig.size());
  bool result = false;
  for (auto lit : orig) {
    uint image = getImage(lit);
    img.push_back(image);
    result = result || image != lit;
  }
  return result;
}

// printing cycles

void Permutation::print(std::ostream& out) {
  for (auto lit : getCycleReprs()) {
    out << "( ";
    vector<uint> cyc;
    getCycle(lit, cyc);
    for (auto s : cyc) {
      out << decode(s) << " ";
    }
    out << ") ";
  }
  out << std::endl;
}

void Permutation::getCycle(uint lit, std::vector<uint>& orb) {
  orb.clear();
  orb.push_back(lit);
  uint sym = getImage(lit);
  while (sym != lit) {
    orb.push_back(sym);
    sym = getImage(sym);
  }
}

bool Permutation::isInvolution() {
  return getMaxCycleSize() == 2;
}

bool Permutation::permutes(uint lit) {
  return perm.count(lit) > 0;
}

uint Permutation::supportSize() {
  return domain.size();
}

bool Permutation::isIdentity() {
  return supportSize() == 0;
}

// TODO: expand this detection method with "non-binary involutions"

bool Permutation::formsMatrixWith(sptr<Permutation> other) {
  if (supportSize() != other->supportSize() || !isInvolution() || !other->isInvolution()) {
    return false;
  }
  for (auto lit : getCycleReprs()) {
    uint sym = perm[lit];
    uint lit_mpd = other->getImage(lit);
    uint sym_mpd = other->getImage(sym);
    if (lit_mpd == sym_mpd ||
            (lit == lit_mpd && sym == sym_mpd) ||
            (lit != lit_mpd && sym != sym_mpd)) {
      return false;
    }
  }
  return true;
}

pair<sptr<Permutation>, sptr<Permutation> > Permutation::getLargest(sptr<Permutation> other) {
  if (other->supportSize() > supportSize()) {
    return
    {
      other, shared_from_this()
    };
  } else {
    return
    {
      shared_from_this(), other
    };
  }
}

void Permutation::getSharedLiterals(sptr<Permutation> other, vector<uint>& shared) {
  shared.clear();
  pair<sptr<Permutation>, sptr<Permutation> > ordered = getLargest(other);
  for (auto lit : ordered.second->posDomain) {
    if (ordered.first->permutes(lit)) {
      shared.push_back(lit);
      shared.push_back(neg(lit));
    }
  }
}

std::vector<uint>& Permutation::getCycleReprs() {
  if (cycleReprs.size() == 0 && supportSize() > 0) { // calculate cycles
    unordered_set<uint> marked;
    for (auto lit : domain) { // TODO: probably possible to replace with more efficient posDomain
      if (marked.count(lit) == 0) {
        cycleReprs.push_back(lit);
        vector<uint> cyc;
        getCycle(lit, cyc);
        for (auto s : cyc) {
          marked.insert(s);
        }
        if (cyc.size() > maxCycleSize) {
          maxCycleSize = cyc.size();
        }
      }
    }
  }
  return cycleReprs;
}

uint Permutation::getMaxCycleSize() {
  if (maxCycleSize == 1) {
    getCycleReprs();
  }
  return maxCycleSize;
}

uint Permutation::getNbCycles() {
  return getCycleReprs().size();
}

bool Permutation::equals(sptr<Permutation> other) {
  if (supportSize() != other->supportSize()
          || getMaxCycleSize() != other->getMaxCycleSize()
          || getNbCycles() != other->getNbCycles()) {
    return false;
  }
  for (uint i = 0; i < supportSize(); ++i) {
    if (image[i] != other->getImage(domain[i])) {
      return false;
    }
  }
  return true;
}

// =========Group=========================

void Group::add(sptr<Permutation> p) {
  permutations.push_back(p);
  support.insert(p->domain.cbegin(), p->domain.cend());
}

void Group::addMatrix(sptr<Matrix> m) {
  matrices.push_back(m);
  cleanPermutations(m);
  for(uint i=0; i<m->nbRows(); ++i){
    auto row = m->getRow(i);
    support.insert(row->cbegin(),row->cend());
  }
  
  if (verbosity > 1) {
    std::clog << "Matrix with " << m->nbRows() << " rows and " << m->nbColumns() << " columns detected" << std::endl;
  }else if (verbosity > 2) {
    m->print(std::clog);
  }
}

void Group::print(std::ostream& out) {
  for (auto p : permutations) {
    p->print(out);
  }
	for (auto m : matrices) {
		m->print(out);
	}
}

// Adds to matrix 3 rows if an initialmatrix is found. 
// The first added row is the shared row.
// The matrix is then maximally extended with new rows given the detected permutations for this group.

sptr<Matrix> Group::getInitialMatrix() {
  std::map<uint, std::vector<sptr<Permutation> > > involutions; // indexed by size
  for (auto p : permutations) {
    if (p->isInvolution()) {
      involutions[p->supportSize()].push_back(p); // so all involutions are at the vector at the index of their size
    }
  }

  sptr<Matrix> result(new Matrix());
  for (auto it = involutions.cbegin(); it != involutions.cend(); ++it) { // looping over all involution sizes
    for (uint i = 0; i < it->second.size(); ++i) {
      for (uint j = i + 1; j < it->second.size(); ++j) { // looping over all unordered pairs of involutions
        if (it->second[i]->formsMatrixWith(it->second[j])) {
          vector<uint>* shared = new vector<uint>();
          it->second[i]->getSharedLiterals(it->second[j], *shared);
          vector<uint>* row_i = new vector<uint>();
          vector<uint>* row_j = new vector<uint>();
          for (auto lit : *shared) {
            row_i->push_back(it->second[i]->getImage(lit));
            row_j->push_back(it->second[j]->getImage(lit));
          }
          result->add(shared);
          result->add(row_i);
          result->add(row_j);
          maximallyExtend(result, 0);
          return result;
        }
      }
    }
  }
  return nullptr;
}

uint Group::getNbMatrices() {
  return matrices.size();
}

uint Group::getNbRowSwaps() {
  uint result = 0;
  for (auto m : matrices) {
    result += m->nbRows()*(m->nbRows() - 1) / 2;
  }
  return result;
}

sptr<Matrix> Group::getMatrix(uint idx){
  return matrices.at(idx);
}

void Group::addMatrices() {
  sptr<Matrix> matrix = getInitialMatrix(); // if possible, gives an initial matrix
  while (matrix != nullptr) {
    uint oldNbRows = 0;
    while (oldNbRows < matrix->nbRows()) {
      // find stabilizer group for all lits but those in the last row of the matrix
      for (uint i = oldNbRows; i < matrix->nbRows() - 1; ++i) {
        // fix all lits but the last row
        theory->getGraph()->setUniqueColor(*matrix->getRow(i));
      }
      oldNbRows = matrix->nbRows();
      std::vector<sptr<Permutation> > symgens;
      theory->getGraph()->getSymmetryGenerators(symgens);
      // now test stabilizer generators on the (former) last row
      for (auto p : symgens) {
        matrix->tryToAddNewRow(p, oldNbRows - 1, theory);
        add(p);
      }
      // for all new rows, test all permutations for this group if they generate a new row
      maximallyExtend(matrix, oldNbRows);
    }
    addMatrix(matrix);
    checkColumnInterchangeability(matrix);
    // fix lits of last row as well
    theory->getGraph()->setUniqueColor(*matrix->getRow(matrix->nbRows() - 1));
    matrix = getInitialMatrix();
  }
}

void Group::checkColumnInterchangeability(sptr<Matrix> m) {
  // create first column
  std::vector<uint>* first = new std::vector<uint>();
  uint firstCol;
  for (firstCol = 0; firstCol < m->nbColumns(); ++firstCol) {
    if (!sign(m->getLit(0, firstCol))) { // found first col starting with positive lit
      for (uint j = 0; j < m->nbRows(); ++j) {
        uint l = m->getLit(j, firstCol);
        first->push_back(l);
        first->push_back(neg(l));
      }
      break;
    }
  }
  sptr<Matrix> newM(new Matrix());
  newM->add(first);

  // test for all swaps of first column with another one
  for (uint i = firstCol + 1; i < m->nbColumns(); ++i) {
    if (!sign(m->getLit(0, i))) { // found other col starting with positive lit
      std::vector<uint>* other = new std::vector<uint>();
      for (uint j = 0; j < m->nbRows(); ++j) {
        uint l = m->getLit(j, i);
        other->push_back(l);
        other->push_back(neg(l));
      }
      // create swap of column i with column firstCol
      Permutation swap(*first, *other);
      if (theory->isSymmetry(swap)) {
        newM->add(other);
      } else {
        delete other;
      }
    }
  }

  if (newM->nbRows() > 2) { // at least 3 rows needed to be of use for symmetry breaking
    addMatrix(newM);
  }
}

void Group::cleanPermutations(sptr<Matrix> matrix) {
  // remove all permutations generated by the current set of matrices
  for (int i = permutations.size() - 1; i >= 0; --i) {
    sptr<Permutation> p = permutations.at(i);
    if (p->supportSize() % matrix->nbColumns() != 0) {
      continue; // can not be a row permutation
    }
    sptr<Permutation> reduct = matrix->testMembership(p);
    if (reduct->isIdentity()) {
      swapErase(permutations, i);
    }
  }
}

void Group::maximallyExtend(sptr<Matrix> matrix, uint indexOfFirstNewRow) {
  for (uint i = indexOfFirstNewRow; i < matrix->nbRows(); ++i) {
    // investigate for new rows
    for (auto p : permutations) {
      matrix->tryToAddNewRow(p, i, theory);
    }
  }
}

uint Group::getSize() {
  return permutations.size();
}

// NOTE: only approximate support for groups with matrices as generators: all matrices are added to the first subgroup
// NOTE: erases permutations for this group

void Group::getDisjointGenerators(std::vector<sptr<Group> >& subgroups) {
  // calculate maximal subsets of generators with pairwise disjoint supports
  subgroups.clear();

  if(matrices.size()>0){
    sptr<Group> current(new Group());
    for(auto m: matrices){
      current->addMatrix(m);
    }
    matrices.clear();
    int previoussize = -1;
    while ((int) current->getSize() > previoussize) {
      previoussize = current->getSize();
      for (int i = permutations.size() - 1; i >= 0; --i) {
        if (not isDisjoint(current->support, permutations[i]->posDomain)) { // it suffices to check that the positive literals occur in the group support
          current->add(permutations[i]);
          swapErase(permutations, i);
        }
      }
    }
    subgroups.push_back(current);     
  }  
  
  while (permutations.size() > 0) {
    uint previoussize = 0;
    sptr<Group> current(new Group());
    current->add(permutations.back());
    permutations.pop_back();
    while (current->getSize() > previoussize) {
      previoussize = current->getSize();
      for (int i = permutations.size() - 1; i >= 0; --i) {
        if (not isDisjoint(current->support, permutations[i]->posDomain)) { // it suffices to check that the positive literals occur in the group support
          current->add(permutations[i]);
          swapErase(permutations, i);
        }
      }
    }
    subgroups.push_back(current);
  }
}

bool Group::permutes(uint lit) {
  return support.count(lit) > 0;
}

uint Group::getSupportSize() {
  return support.size();
}

void eliminateNonStabilizers(std::vector<sptr<Permutation> >& permutations, uint lit) {
  for (uint i = 0; i < permutations.size(); ++i) {
    if (permutations.at(i)->permutes(lit)) {
      permutations[i] = permutations.back();
      permutations.pop_back();
      --i;
    }
  }
}

void getOrbits2(const std::vector<sptr<Permutation> >& permutations, std::vector<sptr<std::vector<uint> > >& orbits) {
  // find positively supported literals
  std::unordered_set<uint> posSupport;
  for (auto p : permutations) {
    for (auto l : p->posDomain) {
      posSupport.insert(l);
    }
  }

  // partition posSupport in orbits
  std::unordered_set<uint> visitedLits;
  for (auto l : posSupport) {
    if (visitedLits.insert(l).second) { // lit did not yet occur in visitedLits
      sptr<std::vector<uint> > newOrbit(new std::vector<uint>());
      newOrbit->push_back(l);
      for (uint i = 0; i < newOrbit->size(); ++i) {
        for (auto p : permutations) {
          uint sym = p->getImage(newOrbit->at(i));
          if (visitedLits.insert(sym).second) {
            newOrbit->push_back(sym);
          }
        }
      }
      orbits.push_back(newOrbit);
    }
  }
}

void getPosLitOccurrenceCount(const std::vector<sptr<Permutation> >& permutations, std::unordered_map<uint, uint>& lits2occ) {
  for (auto p : permutations) {
    for (auto l : p->posDomain) {
      lits2occ[l]++; // using the fact that uint value-initializes to 0
    }
  }
}

void Group::addBinaryClausesTo(Breaker& brkr, std::vector<uint>& out_order, const std::unordered_set<uint>& excludedLits) {
  // now, look for literals with large orbits (of a stabilizer group for smaller literals) as first elements of the order
  std::vector<sptr<Permutation> > perms = permutations;
  while (perms.size() > 0) {
    // as long as we have some permutations stabilizing everything in order so far, continue looking for other literals to add to the order

    // figure out which literal is:
    // 0) a non-excluded variable
    // 1) in _a_ largest orbit with non-excluded variables
    // 2) has the lowest occurrence of literals adhering to 0) and 1)
    std::vector<sptr<std::vector<uint> > > orbs;
    getOrbits2(perms, orbs);
    std::unordered_map<uint, uint> lits2occ;
    getPosLitOccurrenceCount(perms, lits2occ);

    sptr<std::vector<uint> > finalOrb(new std::vector<uint>());
    uint finalLit = UINT_MAX;
    uint finalOccurrence = UINT_MAX;
    for (auto o : orbs) {
      // check whether o is bigger
      if (o->size() < finalOrb->size()) { // note the strict inequality (see condition 1)
        continue;
      }
      // check whether o contains a positive non-excluded lit
      for (auto l : *o) {
        if (excludedLits.count(l) == 0 && lits2occ.count(l) > 0 && lits2occ[l] < finalOccurrence) {
          // success!
          finalLit = l;
          finalOccurrence = lits2occ[l];
          finalOrb = o;
        }
      }
    }
    if (finalOrb->size() > 0) {
      // success!

      // for all literals in its orbit, add binary clause
      for (auto l : *finalOrb) {
        if (l == finalLit) {
          continue;
        }
        brkr.addBinClause(neg(finalLit), l); // add finalLit => l, since there's a symmetry generated by perms that maps finalLit to l
      }

      // add lit to order
      out_order.push_back(finalLit);

      // continue with stabilizer subgroup
      eliminateNonStabilizers(perms, finalLit);
    } else {
      // no more orbits left with positive non-excluded vars
      perms.clear();
    }
  }
}

// NOTE: the order is a list of literals, such that for each literal l, neg(l) is not in the order

void Group::getOrderAndAddBinaryClausesTo(Breaker& brkr, std::vector<uint>& out_order) {
  // first, figure out which literals occur in the matrix, since their order is fixed.
  std::unordered_set<uint> matrixLits;
  for (auto m : matrices) {
    for (uint i = 0; i < m->nbRows(); ++i) {
      for (auto lit : *m->getRow(i)) {
        matrixLits.insert(lit);
      }
    }
  }

  if (useBinaryClauses) {
    if (verbosity > 1) {
      std::clog << "Adding binary symmetry breaking clauses for group..." << std::endl;
    }
    addBinaryClausesTo(brkr, out_order, matrixLits);
  }

  // now, add all literals that are not matrix literals and not yet in the order by their occurrence count
  // first, add ordered lits to matrixLits
  for (auto l : out_order) {
    matrixLits.insert(l);
    matrixLits.insert(neg(l));
  }
  // then create map ordering lits not occurring in matrixLits by their occurrence
  std::unordered_map<uint, uint> lits2occ;
  getPosLitOccurrenceCount(permutations, lits2occ);
  std::multimap<uint, uint> occ2lit;
  for (auto it : lits2occ) {
    if (matrixLits.count(it.first) == 0) {
      occ2lit.insert({it.second, it.first});
    }
  }
  // lastly, add those sorted lits to the order
  for (auto it : occ2lit) {
    out_order.push_back(it.second);
  }

  // ok, all that is left is to add the matrix lits
  matrixLits.clear(); // use this set to avoid double addition of literals occurring in more than one matrix, and avoid addition of negative literals to order
  for (auto m : matrices) {
    for (uint i = 0; i < m->nbRows(); ++i) {
      for (auto l : *m->getRow(i)) {
        if (matrixLits.insert(l).second) {
          matrixLits.insert(neg(l));
          out_order.push_back(l);
          // note how this approach works when matrices contain rows of lits (as opposed to rows of vars)
        }
      }
    }
  }
}

void Group::addBreakingClausesTo(Breaker& brkr) {
  std::vector<uint> order;
  getOrderAndAddBinaryClausesTo(brkr, order);

  if (verbosity > 1) {
    std::clog << "order: ";
    for (auto x : order) {
      std::clog << decode(x) << " ";
    }
    std::clog << endl;
  }

  // add clauses based on detected symmetries
  for (auto p : permutations) {
    brkr.addRegSym(p, order);
  }

  // add clauses based on detected matrices
  for (auto m : matrices) {
    for (uint idx = 0; idx < m->nbRows() - 1; ++idx) {
      sptr<Permutation> rowswap(new Permutation(*m->getRow(idx), *m->getRow(idx + 1)));
      brkr.addRowSym(rowswap, order);
    }
  }
}

// =================MATRIX======================

Matrix::Matrix() {
}

Matrix::~Matrix() {
  for (auto rw : rows) {
    delete rw;
  }
}

void Matrix::print(std::ostream& out) {
  out << "rows " << nbRows() << " columns " << nbColumns() << std::endl;
  for (auto row : rows) {
    for (auto lit : *row) {
      out << decode(lit) << " ";
    }
    out << std::endl;
  }
}

void Matrix::add(std::vector<uint>* row) {
  for (uint i = 0; i < row->size(); ++i) {
    rowco.insert({row->at(i), rows.size()});
    colco.insert({row->at(i), i});
  }
  rows.push_back(row);
}

uint Matrix::nbRows() {
  return rows.size();
}

uint Matrix::nbColumns() {
  if (nbRows() > 0) {
    return rows[0]->size();
  } else {
    return 0;
  }
}

std::vector<uint>* Matrix::getRow(uint rowindex) {
  return rows[rowindex];
}

void Matrix::tryToAddNewRow(sptr<Permutation> p, uint rowIndex, Specification* theory) {
  // checks whether the image of the current row can be used as a new row
  std::vector<uint>* image = new vector<uint>();
  for (auto lit : *getRow(rowIndex)) {
    uint sym = p->getImage(lit);
    if (permutes(sym)) {
      delete image;
      return;
    } else {
      image->push_back(sym);
    }
  }
  // create new row-swapping permutation, test whether it is a symmetry, add the resulting row
  Permutation rowSwap(*getRow(rowIndex), *image);
  if (theory->isSymmetry(rowSwap)) {
    add(image);
  } else {
    delete image;
    return;
  }
}

bool Matrix::permutes(uint x) {
  return rowco.count(x) > 0;
}

uint Matrix::getLit(uint row, uint column) {
  return rows.at(row)->at(column);
}

uint Matrix::getRowNb(uint x) {
  return rowco.at(x);
}

uint Matrix::getColumnNb(uint x) {
  return colco.at(x);
}

sptr<Permutation> Matrix::testMembership(const sptr<Permutation> p) {
  /**
   * NOTE: 
   * We use the first column as base for the matrix.
   * A stabilizer chain then is formed by all submatrices formed by removing the upmost row.
   * A corresponding set of basic orbits is formed by taking as ith orbit (Delta_i) the first elements of row i to row last.
   * A representative function (u_i) for each ith step in the stabilizer chain then maps row j to the swap of row i and j (with j>=i).
   * 
   * We follow here algorithm 2.5.1 from http://www.maths.usyd.edu.au/u/murray/research/essay.pdf
   */

  std::unordered_set<uint> basic_orbit; // Delta^i's
  for (uint i = 0; i < nbRows(); ++i) {
    basic_orbit.insert(getLit(i, 0)); // creating Delta^0
  }
  sptr<Permutation> g = p;
  for (uint l = 0; l < nbRows(); ++l) {
    uint beta_l = getLit(l, 0);
    uint beta_l_g = g->getImage(beta_l);
    if (basic_orbit.count(beta_l_g) == 0) {
      return g; // no permutation generated by this matrix
    } else {
      g = getProductWithRowsWap(g, l, getRowNb(beta_l_g));
      if (g->isIdentity()) {
        return g;
      }
      basic_orbit.erase(beta_l); // creating Delta^{l+1}
    }
  }
  return g;
}

// return p*swap(r1,r2)

sptr<Permutation> Matrix::getProductWithRowsWap(const sptr<Permutation> p, uint r1, uint r2) {
  if (r1 == r2) {
    return p;
  } else if (p->isIdentity()) {
    sptr<Permutation> result(new Permutation(*getRow(r1), *getRow(r2)));
    return result;
  }

  std::vector<std::pair<uint, uint> > protoPerm;

  // add lit-image pairs permuted by rowswap but not by p
  for (uint i = 0; i < nbColumns(); ++i) {
    if (!p->permutes(getLit(r1, i))) {
      protoPerm.push_back({getLit(r1, i), getLit(r2, i)});
    }
    if (!p->permutes(getLit(r2, i))) {
      protoPerm.push_back({getLit(r2, i), getLit(r1, i)});
    }
  }

  // add lit-image pairs permuted by p
  for (uint i = 0; i < p->supportSize(); ++i) {
    uint orig = p->domain.at(i);
    uint img = p->image.at(i);
    if (permutes(img)) {
      uint rowind = getRowNb(img);
      uint colind = getColumnNb(img);
      if (rowind == r1) {
        protoPerm.push_back({orig, getLit(r2, colind)});
      } else if (rowind == r2) {
        protoPerm.push_back({orig, getLit(r1, colind)});
      } else {
        protoPerm.push_back({orig, img});
      }
    } else {
      protoPerm.push_back({orig, img});
    }
  }

  sptr<Permutation> result(new Permutation(protoPerm));
  return result;
}
