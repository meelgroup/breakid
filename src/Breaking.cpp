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
#include <fstream>

#include "Algebraic.hpp"
#include "Breaking.hpp"
#include "Theory.hpp"
#include "global.hpp"

using std::cout;
using std::endl;

Breaker::Breaker(sptr<Specification> origTheo) : originalTheory(origTheo)
{
}

void Breaker::print(std::string& /*origfile*/)
{
    cout << "c number of breaking clauses added: " << getAddedNbClauses()
              << "\n";
    cout << "c max original variable: " << nVars << "\n";
    cout << "c auxiliary variables: " << getAuxiliaryNbVars() << "\n";
    if (not onlyPrintBreakers) {
        cout << "p cnf " << getTotalNbVars() << " " << getTotalNbClauses()
                  << "\n";
        originalTheory->print(cout);
    }
    for (auto c : clauses) {
        c->print(cout);
    }
}

void Breaker::add(sptr<Clause> cl)
{
    clauses.insert(cl);
}

void Breaker::addBinary(uint32_t l1, uint32_t l2)
{
    sptr<Clause> toAdd(new Clause());
    toAdd->lits.push_back(l1);
    toAdd->lits.push_back(l2);
    add(toAdd);
}

void Breaker::addTernary(uint32_t l1, uint32_t l2, uint32_t l3)
{
    sptr<Clause> toAdd(new Clause());
    toAdd->lits.push_back(l1);
    toAdd->lits.push_back(l2);
    toAdd->lits.push_back(l3);
    add(toAdd);
}

void Breaker::addQuaternary(uint32_t l1, uint32_t l2, uint32_t l3, uint32_t l4)
{
    sptr<Clause> toAdd(new Clause());
    toAdd->lits.push_back(l1);
    toAdd->lits.push_back(l2);
    toAdd->lits.push_back(l3);
    toAdd->lits.push_back(l4);
    add(toAdd);
}

void Breaker::addBinClause(uint32_t l1, uint32_t l2)
{
    ++nbBinClauses;
    addBinary(l1, l2);
}

void Breaker::addRegSym(sptr<Permutation> perm, std::vector<uint32_t>& order)
{
    uint32_t current = getTotalNbClauses();
    if (useShatterTranslation) {
        addShatter(perm, order, true);
    } else {
        add(perm, order, true);
    }
    nbRegClauses += getTotalNbClauses() - current;
}

void Breaker::addRowSym(sptr<Permutation> perm, std::vector<uint32_t>& order)
{
    uint32_t current = getTotalNbClauses();
    if (useShatterTranslation) {
        addShatter(perm, order, false);
    } else {
        add(perm, order, false);
    }
    nbRowClauses += getTotalNbClauses() - current;
}

void Breaker::add(sptr<Permutation> perm, std::vector<uint32_t>& order,
                  bool limitExtraConstrs)
{
    std::unordered_set<uint32_t>
        allowedLits; // which are not the last lit in their cycle, unless they map to their negation
    for (uint32_t i = order.size(); i > 0; --i) {
        uint32_t lit = order.at(i - 1);
        if (allowedLits.count(lit) == 0) { // we have a last lit of a cycle
            uint32_t sym = perm->getImage(lit);
            while (
                sym !=
                lit) { // add the other lits of the cycle and the negated cycle
                allowedLits.insert(sym);
                allowedLits.insert(neg(sym));
                sym = perm->getImage(sym);
            }
        }
    }

    int nrExtraConstrs = 0;
    uint32_t prevLit = 0;
    uint32_t prevSym = 0;
    uint32_t prevTst = 0; // previous tseitin
    for (auto l : order) {
        if (limitExtraConstrs && nrExtraConstrs > symBreakingFormLength) {
            break;
        }
        uint32_t sym = perm->getImage(l);
        if (sym != l && allowedLits.count(l)) {
            uint32_t tst = 0;
            if (nrExtraConstrs == 0) {
                // adding clause for l => sym :
                // ~l | sym
                addBinary(neg(l), sym);
            } else if (nrExtraConstrs == 1) {
                // adding clauses for (prevSym => prevLit) => tst and tst => (l => sym)
                tst = getTseitinVar();
                // prevSym | tst
                addBinary(prevSym, tst);
                // ~prevLit | tst
                addBinary(neg(prevLit), tst);
                // ~tst | ~l | sym
                addTernary(neg(tst), neg(l), sym);
                if (useFullTranslation) {
                    // adding clauses for tst => (prevSym => prevLit)
                    // ~tst | ~prevSym | prevLit
                    addTernary(neg(tst), neg(prevSym), prevLit);
                }
            } else {
                // adding clauses for (prevSym => prevLit) & prevTst => tst and tst => (l => sym)
                tst = getTseitinVar();
                // prevSym | ~prevTst | tst
                addTernary(prevSym, neg(prevTst), tst);
                // ~prevLit | ~prevTst | tst
                addTernary(neg(prevLit), neg(prevTst), tst);
                // ~tst | ~l | sym
                addTernary(neg(tst), neg(l), sym);
                if (useFullTranslation) {
                    // adding clauses for tst => prevTst and tst => (prevSym => prevLit)
                    // ~tst | prevTst
                    addBinary(neg(tst), prevTst);
                    // ~tst | ~prevSym | prevLit
                    addTernary(neg(tst), neg(prevSym), prevLit);
                }
            }
            ++nrExtraConstrs;
            if (sym == neg(l)) {
                break;
            }

            prevLit = l;
            prevSym = sym;
            prevTst = tst;
        }
    }
}

void Breaker::addShatter(sptr<Permutation> perm, std::vector<uint32_t>& order,
                         bool limitExtraConstrs)
{
    std::unordered_set<uint32_t>
        allowedLits; // which are not the last lit in their cycle, unless they map to their negation
    for (uint32_t i = order.size(); i > 0; --i) {
        uint32_t lit = order.at(i - 1);
        if (allowedLits.count(lit) == 0) { // we have a last lit of a cycle
            uint32_t sym = perm->getImage(lit);
            while (
                sym !=
                lit) { // add the other lits of the cycle and the negated cycle
                allowedLits.insert(sym);
                allowedLits.insert(neg(sym));
                sym = perm->getImage(sym);
            }
        }
    }

    int nrExtraConstrs = 0;
    uint32_t prevLit = 0;
    uint32_t prevSym = 0;
    uint32_t prevTst = 0; // previous tseitin
    for (auto l : order) {
        if (limitExtraConstrs && nrExtraConstrs > symBreakingFormLength) {
            break;
        }
        uint32_t sym = perm->getImage(l);
        if (sym != l && allowedLits.count(l)) {
            uint32_t tst = 0;
            if (nrExtraConstrs == 0) {
                // adding clause for l => sym :
                // ~l | sym
                addBinary(neg(l), sym);
            } else if (nrExtraConstrs == 1) {
                tst = getTseitinVar();
                // clause(-z, -x, p[x], 0);
                addTernary(neg(prevLit), neg(l), sym);
                // clause(-z, vars+1, 0);
                addBinary(neg(prevLit), tst);
                // clause(p[z], -x, p[x], 0);
                addTernary(prevSym, neg(l), sym);
                // clause(p[z], vars+1, 0);
                addBinary(prevSym, tst);
            } else {
                tst = getTseitinVar();
                // clause(-vars, -z, -x, p[x], 0);
                addQuaternary(neg(prevTst), neg(prevLit), neg(l), sym);
                // clause(-vars, -z, vars+1, 0);
                addTernary(neg(prevTst), neg(prevLit), tst);
                // clause(-vars, p[z], -x, p[x], 0);
                addQuaternary(neg(prevTst), prevSym, neg(l), sym);
                // clause(-vars, p[z], vars+1, 0);
                addTernary(neg(prevTst), prevSym, tst);
            }
            ++nrExtraConstrs;
            if (sym == neg(l)) {
                break;
            }

            prevLit = l;
            prevSym = sym;
            prevTst = tst;
        }
    }
}

uint32_t Breaker::getAuxiliaryNbVars()
{
    return nbExtraVars;
}

uint32_t Breaker::getTotalNbVars()
{
    return nVars + nbExtraVars;
}

uint32_t Breaker::getAddedNbClauses()
{
    return clauses.size();
}

uint32_t Breaker::getTotalNbClauses()
{
    return originalTheory->getSize() + clauses.size();
}

uint32_t Breaker::getNbBinClauses()
{
    return nbBinClauses;
}

uint32_t Breaker::getNbRowClauses()
{
    return nbRowClauses;
}

uint32_t Breaker::getNbRegClauses()
{
    return nbRegClauses;
}

uint32_t Breaker::getTseitinVar()
{
    ++nbExtraVars;
    return encode(getTotalNbVars());
}
