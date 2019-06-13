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

#include "Algebraic.hpp"
#include "Breaking.hpp"
#include "Graph.hpp"
#include "Theory.hpp"

#include <limits.h>
#include <cmath>
#include <cstdio>

using namespace std;

void Permutation::addFromTo(uint32_t from, uint32_t to)
{
    if (from != to
        // only ID's < 2*conf->nVars represent literals
        && from < 2 * conf->nVars
        && to < 2 * conf->nVars
    ) {
        perm[from] = to;
        domain.push_back(from);
        image.push_back(to);
        if (!sign(from)) {
            posDomain.push_back(from);
        }
    }
}

void Permutation::addPrimeSplitToVector(
    vector<shared_ptr<Permutation> >& newPerms)
{
    std::map<size_t, vector<vector<uint32_t> > > lengthToCycles;
    std::map<size_t, std::set<size_t> > primeToPowers;

    //First, we split each cycle up in its relative prime components.
    //E.g., if we have a cycle of length 2^3 * 3 * 5^2
    //We will split it up in 2^3, 3 and 5^2. We also store the information that 2, 3 and 5 are occurring prime components.
    //The prime2powers then containss the information that for prime 2, power 3 occurs, for prime 3, power 1 etcetera.

    for (auto l : getCycleReprs()) {
        vector<uint32_t> cycle;
        getCycle(l, cycle);
        auto cycleSize = cycle.size();
        for (size_t i = 2; i <= cycleSize; i++) {
            size_t nbOcc = 0;
            while (cycleSize % i == 0) {
                nbOcc++;
                cycleSize /= i;
            }
            if (nbOcc != 0) {
                size_t newCycleLength = (size_t)pow(i, nbOcc);
                if (primeToPowers.find(i) == primeToPowers.cend()) {
                    primeToPowers[i] = {};
                }
                primeToPowers[i].insert(nbOcc);
                if (lengthToCycles.find(newCycleLength) ==
                    lengthToCycles.cend()) {
                    lengthToCycles[newCycleLength] = {};
                }
                size_t nbNewCycles = cycle.size() / newCycleLength;
                for (size_t j = 0; j < nbNewCycles; j++) {
                    vector<uint32_t> newCycle;
                    for (size_t k = 0; k < newCycleLength; k++) {
                        newCycle.push_back(cycle[j + nbNewCycles * k]);
                    }
                    lengthToCycles[newCycleLength].push_back(newCycle);
                }
            }
        }
    }

    //Next, we split up the permutation as much as possible.
    //First of all, if there are different relative prime components, we can forget about the original permutation: for each prime component, we keep that generator
    //Next: for each prime component, we also wish to keep "special" generators, namely those that have cycles of equal length. There can be multiple of those.

    for (auto p2p : primeToPowers) {
        auto prime = p2p.first;
        auto powers = p2p.second;

        auto genPerm = std::make_shared<Permutation>(conf);
        size_t smallestPow = SIZE_MAX;
        size_t biggestPow = 0;
        for (auto power : powers) {
            smallestPow = min(power, smallestPow);
            biggestPow = max(power, biggestPow);
            for (auto c : lengthToCycles[pow(prime, power)]) {
                genPerm->addCycle(c);
            }
        }

        newPerms.push_back(genPerm);

        while (smallestPow >= 1) {
            if (smallestPow == biggestPow) {
                smallestPow--;
                continue;
            }
            auto equalCycleGen = std::make_shared<Permutation>(conf);
            for (auto power : powers) {
                for (auto c : lengthToCycles[pow(prime, power)]) {
                    size_t cycleLength = pow(prime, smallestPow);
                    size_t jump = c.size() / cycleLength;
                    for (size_t i = 0; i < jump; i++) {
                        vector<uint32_t> newCycle;
                        for (size_t j = 0; j < cycleLength; j++) {
                            newCycle.push_back(c[i + j * jump]);
                        }
                        equalCycleGen->addCycle(newCycle);
                    }
                }
            }

            newPerms.push_back(equalCycleGen);
            smallestPow--;
        }
    }
}

void Permutation::addCycle(vector<uint32_t>& cyc)
{
    uint32_t n = cyc.size();
    for (uint32_t i = 0; i < n; ++i) {
        addFromTo(cyc.at(i), cyc.at((i + 1) % n));
    }
}

Permutation::Permutation(Config* _conf) :
    conf(_conf)
{
    hash = 0;
    maxCycleSize = 1;
}

Permutation::Permutation(vector<std::pair<uint32_t, uint32_t> >& tuples, Config* _conf) :
    conf(_conf)
{
    for (auto tup : tuples) {
        addFromTo(tup.first, tup.second);
    }
    hash = 0;
    maxCycleSize = 1;
}

Permutation::Permutation(vector<uint32_t>& row1, vector<uint32_t>& row2, Config* _conf) :
    conf(_conf)
{
    for (uint32_t i = 0; i < row1.size() && i < row2.size(); ++i) {
        uint32_t from = row1[i];
        uint32_t to = row2[i];
        addFromTo(from, to);
        addFromTo(to, from);
    }
    hash = 0;
    maxCycleSize = 1;
}

uint32_t Permutation::getImage(uint32_t from)
{
    auto it = perm.find(from);
    if (it != perm.end()) {
        return it->second;
    } else {
        return from;
    }
}

bool Permutation::getImage(vector<uint32_t>& orig, vector<uint32_t>& img)
{
    img.clear();
    img.reserve(orig.size());
    bool result = false;
    for (auto lit : orig) {
        uint32_t image = getImage(lit);
        img.push_back(image);
        result = result || image != lit;
    }
    return result;
}

// printing cycles

void Permutation::print(std::ostream& out)
{
    for (auto lit : getCycleReprs()) {
        out << "( ";
        vector<uint32_t> cyc;
        getCycle(lit, cyc);
        for (auto s : cyc) {
            out << decode(s) << " ";
        }
        out << ") ";
    }
    out << endl;
}

void Permutation::getCycle(uint32_t lit, vector<uint32_t>& orb)
{
    orb.clear();
    orb.push_back(lit);
    uint32_t sym = getImage(lit);
    while (sym != lit) {
        orb.push_back(sym);
        sym = getImage(sym);
    }
}

bool Permutation::isInvolution()
{
    return getMaxCycleSize() == 2;
}

bool Permutation::permutes(uint32_t lit)
{
    return perm.count(lit) > 0;
}

uint32_t Permutation::supportSize()
{
    return domain.size();
}

bool Permutation::isIdentity()
{
    return supportSize() == 0;
}

// TODO: expand this detection method with "non-binary involutions"

bool Permutation::formsMatrixWith(shared_ptr<Permutation> other)
{
    if (supportSize() != other->supportSize() || !isInvolution() ||
        !other->isInvolution()) {
        return false;
    }
    for (auto lit : getCycleReprs()) {
        uint32_t sym = perm[lit];
        uint32_t lit_mpd = other->getImage(lit);
        uint32_t sym_mpd = other->getImage(sym);
        if (lit_mpd == sym_mpd || (lit == lit_mpd && sym == sym_mpd) ||
            (lit != lit_mpd && sym != sym_mpd)) {
            return false;
        }
    }
    return true;
}

pair<shared_ptr<Permutation>, shared_ptr<Permutation> > Permutation::getLargest(
    shared_ptr<Permutation> other)
{
    if (other->supportSize() > supportSize()) {
        return {other, shared_from_this()};
    } else {
        return {shared_from_this(), other};
    }
}

void Permutation::getSharedLiterals(shared_ptr<Permutation> other,
                                    vector<uint32_t>& shared)
{
    shared.clear();
    pair<shared_ptr<Permutation>, shared_ptr<Permutation> > ordered = getLargest(other);
    for (auto lit : ordered.second->posDomain) {
        if (ordered.first->permutes(lit)) {
            shared.push_back(lit);
            shared.push_back(neg(lit));
        }
    }
}

vector<uint32_t>& Permutation::getCycleReprs()
{
    if (cycleReprs.size() == 0 && supportSize() > 0) { // calculate cycles
        unordered_set<uint32_t> marked;
        for (
            auto lit :
            domain) { // TODO: probably possible to replace with more efficient posDomain
            if (marked.count(lit) == 0) {
                cycleReprs.push_back(lit);
                vector<uint32_t> cyc;
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

uint32_t Permutation::getMaxCycleSize()
{
    if (maxCycleSize == 1) {
        getCycleReprs();
    }
    return maxCycleSize;
}

uint32_t Permutation::getNbCycles()
{
    return getCycleReprs().size();
}

bool Permutation::equals(shared_ptr<Permutation> other)
{
    if (supportSize() != other->supportSize() ||
        getMaxCycleSize() != other->getMaxCycleSize() ||
        getNbCycles() != other->getNbCycles()) {
        return false;
    }
    for (uint32_t i = 0; i < supportSize(); ++i) {
        if (image[i] != other->getImage(domain[i])) {
            return false;
        }
    }
    return true;
}

// =========Group=========================

Group::Group(Config* _conf) :
    conf(_conf)
{}

void Group::add(shared_ptr<Permutation> p)
{
    permutations.push_back(p);
    support.insert(p->domain.cbegin(), p->domain.cend());
}

void Group::addMatrix(shared_ptr<Matrix> m)
{
    matrices.push_back(m);
    cleanPermutations(m);
    for (uint32_t i = 0; i < m->nbRows(); ++i) {
        auto row = m->getRow(i);
        support.insert(row->cbegin(), row->cend());
    }

    if (conf->verbosity > 1) {
        cout << "Matrix with " << m->nbRows() << " rows and "
                  << m->nbColumns() << " columns detected" << endl;
    } else if (conf->verbosity > 2) {
        m->print(cout);
    }
}

void Group::print(std::ostream& out)
{
    cout << "-- Permutations:" << endl;
    for (auto p : permutations) {
        p->print(out);
    }
    cout << "-- Matrices:" << endl;
    for (auto m : matrices) {
        m->print(out);
    }
}

// Adds to matrix 3 rows if an initialmatrix is found.
// The first added row is the shared row.
// The matrix is then maximally extended with new rows given the detected permutations for this group.

shared_ptr<Matrix> Group::getInitialMatrix()
{
    std::map<uint32_t, vector<shared_ptr<Permutation> > >
        involutions; // indexed by size
    for (auto p : permutations) {
        if (p->isInvolution()) {
            involutions[p->supportSize()].push_back(
                p); // so all involutions are at the vector at the index of their size
        }
    }

    shared_ptr<Matrix> result(new Matrix(conf));
    for (auto it = involutions.cbegin(); it != involutions.cend();
         ++it) { // looping over all involution sizes
        for (uint32_t i = 0; i < it->second.size(); ++i) {
            for (uint32_t j = i + 1; j < it->second.size();
                 ++j) { // looping over all unordered pairs of involutions
                if (it->second[i]->formsMatrixWith(it->second[j])) {
                    vector<uint32_t>* shared = new vector<uint32_t>();
                    it->second[i]->getSharedLiterals(it->second[j], *shared);
                    vector<uint32_t>* row_i = new vector<uint32_t>();
                    vector<uint32_t>* row_j = new vector<uint32_t>();
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

uint32_t Group::getNbMatrices()
{
    return matrices.size();
}

uint32_t Group::getNbRowSwaps()
{
    uint32_t result = 0;
    for (auto m : matrices) {
        result += m->nbRows() * (m->nbRows() - 1) / 2;
    }
    return result;
}

shared_ptr<Matrix> Group::getMatrix(uint32_t idx)
{
    return matrices.at(idx);
}

void Group::addMatrices()
{
    ///if possible, gives an initial matrix
    shared_ptr<Matrix> matrix = getInitialMatrix();

    while (matrix != nullptr) {
        uint32_t oldNbRows = 0;
        while (oldNbRows < matrix->nbRows()) {
            // find stabilizer group for all lits but those in the last row of the matrix
            for (uint32_t i = oldNbRows; i < matrix->nbRows() - 1; ++i) {
                // fix all lits but the last row
                theory->getGraph()->setUniqueColor(*matrix->getRow(i));
            }
            oldNbRows = matrix->nbRows();
            vector<shared_ptr<Permutation> > symgens;
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
        theory->getGraph()->setUniqueColor(
            *matrix->getRow(matrix->nbRows() - 1));
        matrix = getInitialMatrix();
    }
}

void Group::checkColumnInterchangeability(shared_ptr<Matrix> m)
{
    // create first column
    vector<uint32_t>* first = new vector<uint32_t>();
    uint32_t firstCol;
    for (firstCol = 0; firstCol < m->nbColumns(); ++firstCol) {
        if (!sign(m->getLit(
                0, firstCol))) { // found first col starting with positive lit
            for (uint32_t j = 0; j < m->nbRows(); ++j) {
                uint32_t l = m->getLit(j, firstCol);
                first->push_back(l);
                first->push_back(neg(l));
            }
            break;
        }
    }
    shared_ptr<Matrix> newM(new Matrix(conf));
    newM->add(first);

    // test for all swaps of first column with another one
    for (uint32_t i = firstCol + 1; i < m->nbColumns(); ++i) {
        if (!sign(m->getLit(0,
                            i))) { // found other col starting with positive lit
            vector<uint32_t>* other = new vector<uint32_t>();
            for (uint32_t j = 0; j < m->nbRows(); ++j) {
                uint32_t l = m->getLit(j, i);
                other->push_back(l);
                other->push_back(neg(l));
            }
            // create swap of column i with column firstCol
            Permutation swap(*first, *other, conf);
            if (theory->isSymmetry(swap)) {
                newM->add(other);
            } else {
                delete other;
            }
        }
    }

    if (newM->nbRows() >
        2) { // at least 3 rows needed to be of use for symmetry breaking
        addMatrix(newM);
    }
}

void Group::cleanPermutations(shared_ptr<Matrix> matrix)
{
    // remove all permutations generated by the current set of matrices
    for (int i = permutations.size() - 1; i >= 0; --i) {
        shared_ptr<Permutation> p = permutations.at(i);
        if (p->supportSize() % matrix->nbColumns() != 0) {
            continue; // can not be a row permutation
        }
        shared_ptr<Permutation> reduct = matrix->testMembership(p);
        if (reduct->isIdentity()) {
            swapErase(permutations, i);
        }
    }
}

void Group::maximallyExtend(shared_ptr<Matrix> matrix, uint32_t indexOfFirstNewRow)
{
    for (uint32_t i = indexOfFirstNewRow; i < matrix->nbRows(); ++i) {
        // investigate for new rows
        for (auto p : permutations) {
            matrix->tryToAddNewRow(p, i, theory);
        }
    }
}

uint32_t Group::getSize()
{
    return permutations.size();
}

// NOTE: only approximate support for groups with matrices as generators: all matrices are added to the first subgroup
// NOTE: erases permutations for this group

void Group::getDisjointGenerators(vector<shared_ptr<Group> >& subgroups)
{
    // calculate maximal subsets of generators with pairwise disjoint supports
    subgroups.clear();

    if (matrices.size() > 0) {
        shared_ptr<Group> current(new Group(conf));
        for (auto m : matrices) {
            current->addMatrix(m);
        }
        matrices.clear();
        int previoussize = -1;
        while ((int)current->getSize() > previoussize) {
            previoussize = current->getSize();
            for (int i = permutations.size() - 1; i >= 0; --i) {
                if (!isDisjoint(
                        current->support,
                        permutations[i]
                            ->posDomain)) { // it suffices to check that the positive literals occur in the group support
                    current->add(permutations[i]);
                    swapErase(permutations, i);
                }
            }
        }
        subgroups.push_back(current);
    }

    while (permutations.size() > 0) {
        uint32_t previoussize = 0;
        shared_ptr<Group> current(new Group(conf));
        current->add(permutations.back());
        permutations.pop_back();
        while (current->getSize() > previoussize) {
            previoussize = current->getSize();
            for (int i = permutations.size() - 1; i >= 0; --i) {
                if (!isDisjoint(
                        current->support,
                        permutations[i]
                            ->posDomain)) { // it suffices to check that the positive literals occur in the group support
                    current->add(permutations[i]);
                    swapErase(permutations, i);
                }
            }
        }
        subgroups.push_back(current);
    }
}

bool Group::permutes(uint32_t lit)
{
    return support.count(lit) > 0;
}

uint32_t Group::getSupportSize()
{
    return support.size();
}

void eliminateNonStabilizers(vector<shared_ptr<Permutation> >& permutations,
                             uint32_t lit)
{
    for (uint32_t i = 0; i < permutations.size(); ++i) {
        if (permutations.at(i)->permutes(lit)) {
            permutations[i] = permutations.back();
            permutations.pop_back();
            --i;
        }
    }
}

void getOrbits2(const vector<shared_ptr<Permutation> >& permutations,
                vector<shared_ptr<vector<uint32_t> > >& orbits)
{
    // find positively supported literals
    std::unordered_set<uint32_t> posSupport;
    for (auto p : permutations) {
        for (auto l : p->posDomain) {
            posSupport.insert(l);
        }
    }

    // partition posSupport in orbits
    std::unordered_set<uint32_t> visitedLits;
    for (auto l : posSupport) {
        if (visitedLits.insert(l)
                .second) { // lit did not yet occur in visitedLits
            shared_ptr<vector<uint32_t> > newOrbit(new vector<uint32_t>());
            newOrbit->push_back(l);
            for (uint32_t i = 0; i < newOrbit->size(); ++i) {
                for (auto p : permutations) {
                    uint32_t sym = p->getImage(newOrbit->at(i));
                    if (visitedLits.insert(sym).second) {
                        newOrbit->push_back(sym);
                    }
                }
            }
            orbits.push_back(newOrbit);
        }
    }
}

void getPosLitOccurrenceCount(
    const vector<shared_ptr<Permutation> >& permutations,
    std::unordered_map<uint32_t, uint32_t>& lits2occ)
{
    for (auto p : permutations) {
        for (auto l : p->posDomain) {
            lits2occ[l]++; // using the fact that uint32_t value-initializes to 0
        }
    }
}

void Group::addBinaryClausesTo(Breaker& brkr, vector<uint32_t>& out_order,
                               const std::unordered_set<uint32_t>& excludedLits)
{
    // now, look for literals with large orbits (of a stabilizer group for smaller literals) as first elements of the order
    vector<shared_ptr<Permutation> > perms = permutations;
    while (perms.size() > 0) {
        // as long as we have some permutations stabilizing everything in order so far, continue looking for other literals to add to the order

        // figure out which literal is:
        // 0) a non-excluded variable
        // 1) in _a_ largest orbit with non-excluded variables
        // 2) has the lowest occurrence of literals adhering to 0) and 1)
        vector<shared_ptr<vector<uint32_t> > > orbs;
        getOrbits2(perms, orbs);
        std::unordered_map<uint32_t, uint32_t> lits2occ;
        getPosLitOccurrenceCount(perms, lits2occ);

        shared_ptr<vector<uint32_t> > finalOrb(new vector<uint32_t>());
        uint32_t finalLit = UINT_MAX;
        uint32_t finalOccurrence = UINT_MAX;
        for (auto o : orbs) {
            // check whether o is bigger
            if (o->size() <
                finalOrb
                    ->size()) { // note the strict inequality (see condition 1)
                continue;
            }
            // check whether o contains a positive non-excluded lit
            for (auto l : *o) {
                if (excludedLits.count(l) == 0 && lits2occ.count(l) > 0 &&
                    lits2occ[l] < finalOccurrence) {
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
                brkr.addBinClause(
                    neg(finalLit),
                    l); // add finalLit => l, since there's a symmetry generated by perms that maps finalLit to l
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

void Group::getOrderAndAddBinaryClausesTo(Breaker& brkr,
                                          vector<uint32_t>& out_order)
{
    // first, figure out which literals occur in the matrix, since their order is fixed.
    std::unordered_set<uint32_t> matrixLits;
    for (auto m : matrices) {
        for (uint32_t i = 0; i < m->nbRows(); ++i) {
            for (auto lit : *m->getRow(i)) {
                matrixLits.insert(lit);
            }
        }
    }

    if (conf->useBinaryClauses) {
        if (conf->verbosity > 1) {
            cout << "Adding binary symmetry breaking clauses for group..."
                      << endl;
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
    std::unordered_map<uint32_t, uint32_t> lits2occ;
    getPosLitOccurrenceCount(permutations, lits2occ);
    std::multimap<uint32_t, uint32_t> occ2lit;
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
    matrixLits
        .clear(); // use this set to avoid double addition of literals occurring in more than one matrix, and avoid addition of negative literals to order
    for (auto m : matrices) {
        for (uint32_t i = 0; i < m->nbRows(); ++i) {
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

void Group::addBreakingClausesTo(Breaker& brkr)
{
    vector<uint32_t> order;
    getOrderAndAddBinaryClausesTo(brkr, order);

    if (conf->verbosity > 3) {
        cout << "order: ";
        for (auto x : order) {
            cout << decode(x) << " ";
        }
        cout << endl;
    }

    // add clauses based on detected symmetries
    for (auto p : permutations) {
        brkr.addRegSym(p, order);
    }

    // add clauses based on detected matrices
    for (auto m : matrices) {
        for (uint32_t idx = 0; idx < m->nbRows() - 1; ++idx) {
            shared_ptr<Permutation> rowswap(
                new Permutation(*m->getRow(idx), *m->getRow(idx + 1), conf));
            brkr.addRowSym(rowswap, order);
        }
    }
}

// =================MATRIX======================

Matrix::Matrix(Config* _conf) :
    conf(_conf)
{
}

Matrix::~Matrix()
{
    for (auto rw : rows) {
        delete rw;
    }
}

void Matrix::print(std::ostream& out)
{
    out << "rows " << nbRows() << " columns " << nbColumns() << endl;
    for (auto row : rows) {
        for (auto lit : *row) {
            out << decode(lit) << " ";
        }
        out << endl;
    }
}

void Matrix::add(vector<uint32_t>* row)
{
    for (uint32_t i = 0; i < row->size(); ++i) {
        rowco.insert({row->at(i), rows.size()});
        colco.insert({row->at(i), i});
    }
    rows.push_back(row);
}

uint32_t Matrix::nbRows()
{
    return rows.size();
}

uint32_t Matrix::nbColumns()
{
    if (nbRows() > 0) {
        return rows[0]->size();
    } else {
        return 0;
    }
}

vector<uint32_t>* Matrix::getRow(uint32_t rowindex)
{
    return rows[rowindex];
}

void Matrix::tryToAddNewRow(shared_ptr<Permutation> p, uint32_t rowIndex,
                            Specification* theory)
{
    // checks whether the image of the current row can be used as a new row
    vector<uint32_t>* image = new vector<uint32_t>();
    for (auto lit : *getRow(rowIndex)) {
        uint32_t sym = p->getImage(lit);
        if (permutes(sym)) {
            delete image;
            return;
        } else {
            image->push_back(sym);
        }
    }
    // create new row-swapping permutation, test whether it is a symmetry, add the resulting row
    Permutation rowSwap(*getRow(rowIndex), *image, conf);
    if (theory->isSymmetry(rowSwap)) {
        add(image);
    } else {
        delete image;
        return;
    }
}

bool Matrix::permutes(uint32_t x)
{
    return rowco.count(x) > 0;
}

uint32_t Matrix::getLit(uint32_t row, uint32_t column)
{
    return rows.at(row)->at(column);
}

uint32_t Matrix::getRowNb(uint32_t x)
{
    return rowco.at(x);
}

uint32_t Matrix::getColumnNb(uint32_t x)
{
    return colco.at(x);
}

shared_ptr<Permutation> Matrix::testMembership(const shared_ptr<Permutation> p)
{
    /**
   * NOTE: 
   * We use the first column as base for the matrix.
   * A stabilizer chain then is formed by all submatrices formed by removing the upmost row.
   * A corresponding set of basic orbits is formed by taking as ith orbit (Delta_i) the first elements of row i to row last.
   * A representative function (u_i) for each ith step in the stabilizer chain then maps row j to the swap of row i and j (with j>=i).
   * 
   * We follow here algorithm 2.5.1 from http://www.maths.usyd.edu.au/u/murray/research/essay.pdf
   */

    std::unordered_set<uint32_t> basic_orbit; // Delta^i's
    for (uint32_t i = 0; i < nbRows(); ++i) {
        basic_orbit.insert(getLit(i, 0)); // creating Delta^0
    }
    shared_ptr<Permutation> g = p;
    for (uint32_t l = 0; l < nbRows(); ++l) {
        uint32_t beta_l = getLit(l, 0);
        uint32_t beta_l_g = g->getImage(beta_l);
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

shared_ptr<Permutation> Matrix::getProductWithRowsWap(const shared_ptr<Permutation> p,
                                                uint32_t r1, uint32_t r2)
{
    if (r1 == r2) {
        return p;
    } else if (p->isIdentity()) {
        shared_ptr<Permutation> result(new Permutation(*getRow(r1), *getRow(r2), conf));
        return result;
    }

    vector<std::pair<uint32_t, uint32_t> > protoPerm;

    // add lit-image pairs permuted by rowswap but not by p
    for (uint32_t i = 0; i < nbColumns(); ++i) {
        if (!p->permutes(getLit(r1, i))) {
            protoPerm.push_back({getLit(r1, i), getLit(r2, i)});
        }
        if (!p->permutes(getLit(r2, i))) {
            protoPerm.push_back({getLit(r2, i), getLit(r1, i)});
        }
    }

    // add lit-image pairs permuted by p
    for (uint32_t i = 0; i < p->supportSize(); ++i) {
        uint32_t orig = p->domain.at(i);
        uint32_t img = p->image.at(i);
        if (permutes(img)) {
            uint32_t rowind = getRowNb(img);
            uint32_t colind = getColumnNb(img);
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

    shared_ptr<Permutation> result(new Permutation(protoPerm, conf));
    return result;
}
