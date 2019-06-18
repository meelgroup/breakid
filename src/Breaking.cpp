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

using std::cout;
using std::endl;

Breaker::Breaker(OnlCNF const* origTheo, Config* _conf) :
    originalTheory(origTheo)
    , conf(_conf)
{
}

void Breaker::print(bool only_breakers)
{
    cout << "c number of breaking clauses added: " << getAddedNbClauses()
              << "\n";
    cout << "c max original variable: " << conf->nVars << "\n";
    cout << "c auxiliary variables: " << getAuxiliaryNbVars() << "\n";
    if (!only_breakers) {
        cout << "p cnf " << getTotalNbVars() << " " << getTotalNbClauses()
                  << "\n";
        originalTheory->print(cout);
    }
    for (auto c : clauses) {
        c->print(cout);
    }
}

vector<vector<BID::BLit>> Breaker::get_brk_cls()
{
    vector<vector<BID::BLit>> cls;
    for (const auto& cl: clauses) {
        cls.push_back(cl->lits);
    }
    return cls;
}

void Breaker::add(shared_ptr<Clause> cl)
{
    clauses.insert(cl);
}

void Breaker::addBinary(BLit l1, BLit l2)
{
    shared_ptr<Clause> toAdd(new Clause());
    toAdd->lits.push_back(l1);
    toAdd->lits.push_back(l2);
    add(toAdd);
}

void Breaker::addTernary(BLit l1, BLit l2, BLit l3)
{
    shared_ptr<Clause> toAdd(new Clause());
    toAdd->lits.push_back(l1);
    toAdd->lits.push_back(l2);
    toAdd->lits.push_back(l3);
    add(toAdd);
}

void Breaker::addQuaternary(BLit l1, BLit l2, BLit l3, BLit l4)
{
    shared_ptr<Clause> toAdd(new Clause());
    toAdd->lits.push_back(l1);
    toAdd->lits.push_back(l2);
    toAdd->lits.push_back(l3);
    toAdd->lits.push_back(l4);
    add(toAdd);
}

void Breaker::addBinClause(BLit l1, BLit l2)
{
    ++nbBinClauses;
    addBinary(l1, l2);
}

void Breaker::addSym(
    shared_ptr<Permutation> perm
    , std::vector<BLit>& order
    , bool limitExtraConstrs
) {
    uint32_t current = getTotalNbClauses();
    if (conf->useShatterTranslation) {
        assert(false);
        addShatter(perm, order, limitExtraConstrs);
    } else {
        addBreakID(perm, order, limitExtraConstrs);
    }
    nbRegClauses += getTotalNbClauses() - current;
}

void Breaker::addBreakID(shared_ptr<Permutation> perm, std::vector<BLit>& order,
                  bool limitExtraConstrs)
{
    /// which are not the last lit in their cycle, unless they map to their negation
    std::unordered_set<BLit> allowedLits;

    for (uint32_t i = order.size(); i > 0; --i) {
        BLit lit = order.at(i - 1);
        if (allowedLits.count(lit) == 0) { // we have a last lit of a cycle
            BLit sym = perm->getImage(lit);

            // add the other lits of the cycle and the negated cycle
            while (sym != lit) {
                allowedLits.insert(sym);
                allowedLits.insert(~sym);
                sym = perm->getImage(sym);
            }
        }
    }

    int nrExtraConstrs = 0;
    BLit prevLit = BLit_Undef;
    BLit prevSym = BLit_Undef;
    BLit prevTst = BLit_Undef; // previous tseitin
    for (auto l : order) {
        if (limitExtraConstrs && nrExtraConstrs > conf->symBreakingFormLength) {
            break;
        }
        BLit sym = perm->getImage(l);
        if (sym != l && allowedLits.count(l)) {
            BLit tst = BLit_Undef;
            if (nrExtraConstrs == 0) {
                // adding clause for l => sym :
                // ~l | sym
                addBinary(~l, sym);
            } else if (nrExtraConstrs == 1) {
                // adding clauses for (prevSym => prevLit) => tst and tst => (l => sym)
                tst = getTseitinVar();
                // prevSym | tst
                addBinary(prevSym, tst);
                // ~prevLit | tst
                addBinary(~prevLit, tst);
                // ~tst | ~l | sym
                addTernary(~tst, ~l, sym);
                if (conf->useFullTranslation) {
                    // adding clauses for tst => (prevSym => prevLit)
                    // ~tst | ~prevSym | prevLit
                    addTernary(~tst, ~prevSym, prevLit);
                }
            } else {
                // adding clauses for (prevSym => prevLit) & prevTst => tst and tst => (l => sym)
                tst = getTseitinVar();
                // prevSym | ~prevTst | tst
                addTernary(prevSym, ~prevTst, tst);
                // ~prevLit | ~prevTst | tst
                addTernary(~prevLit, ~prevTst, tst);
                // ~tst | ~l | sym
                addTernary(~tst, ~l, sym);
                if (conf->useFullTranslation) {
                    // adding clauses for tst => prevTst and tst => (prevSym => prevLit)
                    // ~tst | prevTst
                    addBinary(~tst, prevTst);
                    // ~tst | ~prevSym | prevLit
                    addTernary(~tst, ~prevSym, prevLit);
                }
            }
            ++nrExtraConstrs;
            if (sym == ~l) {
                break;
            }

            prevLit = l;
            prevSym = sym;
            prevTst = tst;
        }
    }
}

void Breaker::addShatter(shared_ptr<Permutation> perm, std::vector<BLit>& order,
                         bool limitExtraConstrs)
{
    // which are not the last lit in their cycle, unless they map to their negation
    std::unordered_set<BLit> allowedLits;

    for (uint32_t i = order.size(); i > 0; --i) {
        BLit lit = order.at(i - 1);
        if (allowedLits.count(lit) == 0) { // we have a last lit of a cycle
            BLit sym = perm->getImage(lit);

            // add the other lits of the cycle and the negated cycle
            while (sym != lit) {
                allowedLits.insert(sym);
                allowedLits.insert(~sym);
                sym = perm->getImage(sym);
            }
        }
    }

    int nrExtraConstrs = 0;
    BLit prevLit = BLit_Undef;
    BLit prevSym = BLit_Undef;
    BLit prevTst = BLit_Undef; // previous tseitin
    for (auto l : order) {
        if (limitExtraConstrs && nrExtraConstrs > conf->symBreakingFormLength) {
            break;
        }
        BLit sym = perm->getImage(l);
        if (sym != l && allowedLits.count(l)) {
            BLit tst = BLit_Undef;
            if (nrExtraConstrs == 0) {
                // adding clause for l => sym :
                // ~l | sym
                addBinary(~l, sym);
            } else if (nrExtraConstrs == 1) {
                tst = getTseitinVar();
                // clause(-z, -x, p[x], 0);
                addTernary(~prevLit, ~l, sym);
                // clause(-z, vars+1, 0);
                addBinary(~prevLit, tst);
                // clause(p[z], -x, p[x], 0);
                addTernary(prevSym, ~l, sym);
                // clause(p[z], vars+1, 0);
                addBinary(prevSym, tst);
            } else {
                tst = getTseitinVar();
                // clause(-vars, -z, -x, p[x], 0);
                addQuaternary(~prevTst, ~prevLit, ~l, sym);
                // clause(-vars, -z, vars+1, 0);
                addTernary(~prevTst, ~prevLit, tst);
                // clause(-vars, p[z], -x, p[x], 0);
                addQuaternary(~prevTst, prevSym, ~l, sym);
                // clause(-vars, p[z], vars+1, 0);
                addTernary(~prevTst, prevSym, tst);
            }
            ++nrExtraConstrs;
            if (sym == ~l) {
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
    return conf->nVars + nbExtraVars;
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

BLit Breaker::getTseitinVar()
{
    BLit ret = BLit(getTotalNbVars(), false);
    ++nbExtraVars;

    return ret;
}
