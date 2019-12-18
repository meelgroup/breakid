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

#include "Theory.hpp"
#include <algorithm>
#include <bitset>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string.h>
#include "Algebraic.hpp"
#include "Breaking.hpp"
#include "Graph.hpp"
#include "breakid/solvertypesmini.hpp"

using std::cout;
using std::endl;
using std::set;
using std::ifstream;
using std::string;
using std::make_shared;
using std::istringstream;
using std::stringstream;


OnlCNF::OnlCNF(Config* _conf) :
    conf(_conf)
{
    graph = new Graph(conf);
}

OnlCNF::~OnlCNF()
{
    delete group;
    group = NULL;

    delete graph;
    graph = NULL;
}

void OnlCNF::add_clause(BID::BLit* lits, uint32_t size)
{
    graph->add_clause(lits, size);
    cl_sizes.push_back(size);
    for(uint32_t i = 0; i < size; i++) {
        cl_lits.push_back(lits[i]);
    }
}

void OnlCNF::end_dynamic_cnf()
{
    graph->end_dynamic_cnf();

    if (conf->verbosity) {
        cout << "-> Detecting symmetry group..." << endl;
    }
}

void OnlCNF::set_new_group() {
    group = new Group(conf);
    vector<shared_ptr<Permutation> > symgens;
    graph->getSymmetryGenerators(symgens, conf->steps_lim, &conf->remain_steps_lim);
    for (auto symgen : symgens) {
        group->add(symgen);
    }
}

void OnlCNF::set_old_group(Group* grp) {
    group = grp;
    for (uint32_t l = 0; l < 2 * conf->nVars; ++l) {
        if (!grp->permutes(BLit::toBLit(l))) {
            graph->setUniqueColor(l);
        }
    }
    for (uint32_t m = 0; m < grp->getNbMatrices(); ++m) {
        auto mat = grp->getMatrix(m);
        for (uint32_t r = 0; r < mat->nbRows() - 1; ++r) {
            graph->setUniqueColor(*mat->getRow(r));
        }
    }
}

void OnlCNF::print(std::ostream&) const
{
    assert(false);
}

uint32_t OnlCNF::getSize() const
{
    return num_cls;
}

///Find set of clauses that subgroup permutates
///TODO fix this is super slow!!!!
void OnlCNF::setSubTheory(Group* subgroup)
{
    //WARNING What is this method supposed to do: keep all clauses that are not mapped to themselves?
    //WARNING Is it simply made approximative on purpose or by accident?

    OnlCNF* cnf = new OnlCNF(conf);
    size_t at = 0;
    for (const uint32_t sz : cl_sizes) {
        for (uint32_t i = at; i < sz+at; i++) {
            const BLit lit = cl_lits[i];
            if (subgroup->permutes(lit)) {
                cnf->add_clause(cl_lits.data()+at, sz);
                break;
            }
        }
        at+=sz;
    }
    cnf->set_old_group(subgroup);
    subgroup->theory = cnf;
}

bool OnlCNF::isSymmetry(Permutation& prm)
{
    //Empty set of clauses, return true
    if (cl_sizes.empty()) {
        return true;
    }

    //Fill clauses if it's not been filled yet
    if (clauses.empty()) {
        size_t at = 0;
        for (const uint32_t sz : cl_sizes) {
            BLit* lits = cl_lits.data()+at;
            std::sort(lits, lits+sz);

            MyCl cl;
            cl.lits = lits;
            cl.sz = sz;
            cl.hashValue = _getHash(lits, sz);
            clauses.insert(cl);

            at+=sz;
        }
    }

    //Every change clause's image MUST exist in the clause database
    size_t at = 0;
    vector<BLit> symmetrical;
    for (const uint32_t sz : cl_sizes) {
        BLit* lits = cl_lits.data()+at;
        at+=sz;

        if (!prm.getImage(lits, sz, symmetrical)) {
            continue;
        }
        std::sort(symmetrical.begin(), symmetrical.end());
        MyCl symcl;
        symcl.lits = symmetrical.data();
        symcl.sz = symmetrical.size();
        symcl.hashValue = _getHash(symmetrical);

        if (clauses.count(symcl) == 0) {
            return false;
        }
    }
    return true;
}

const Graph* OnlCNF::getGraph() const
{
    return graph;
}
