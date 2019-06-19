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

#include "breakid.hpp"
#include "Breaking.hpp"
#include "Theory.hpp"
#include "Graph.hpp"
#include "config.hpp"
#include "breakid/solvertypesmini.hpp"
#include "GitSHA1.h"

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <sstream>
#include <iostream>

using std::cout;
using std::endl;
using std::string;
using std::ofstream;
using std::stringstream;
using std::istringstream;
using std::make_shared;
using std::vector;
using namespace BID;

struct BID::PrivateData
{
    vector<Group*> subgroups;
    uint32_t totalNbMatrices = 0;
    uint32_t totalNbRowSwaps = 0;

    OnlCNF* theory = NULL;
    Breaker* brkr = NULL;
    Config* conf = NULL;

    ~PrivateData()
    {
        for(auto& sg: subgroups) {
            delete sg;
        }
        subgroups.clear();

        delete theory;
        delete conf;
        delete brkr;
    }
};

BreakID::~BreakID()
{
    delete dat;
}

BreakID::BreakID()
{
    dat = new PrivateData;
    dat->conf = new Config;
}

void BreakID::set_useMatrixDetection(bool val)
{
    dat->conf->useMatrixDetection = val;
}

void BreakID::set_useBinaryClauses(bool val)
{
    dat->conf->useBinaryClauses = val;
}

void BreakID::set_useShatterTranslation(bool val)
{
    dat->conf->useShatterTranslation = val;
}

void BreakID::set_useFullTranslation(bool val)
{
    dat->conf->useFullTranslation = val;
}

void BreakID::set_symBreakingFormLength(int val)
{
    dat->conf->symBreakingFormLength = val;
}

void BreakID::set_verbosity(uint32_t val)
{
    dat->conf->verbosity = val;
}

void BreakID::set_steps_lim(int64_t val)
{
    dat->conf->steps_lim = val;
}

int64_t BreakID::get_steps_remain() const
{
    return dat->conf->remain_steps_lim;
}

void BreakID::start_dynamic_cnf(uint32_t nVars)
{
    assert(dat->theory == NULL);
    dat->conf->nVars = nVars;
    dat->theory = new OnlCNF(dat->conf);
}

void BreakID::add_clause(BID::BLit* start, size_t num)
{
    dat->theory->add_clause(start, num);
}

void BreakID::end_dynamic_cnf()
{
    dat->theory->end_dynamic_cnf();
    dat->theory->set_new_group();
}

uint32_t BreakID::get_num_generators()
{
    return dat->theory->group->getSize();
}

void BreakID::detect_subgroups() {
    dat->theory->group->getDisjointGenerators(dat->subgroups);
}

uint64_t BreakID::get_num_subgroups() const
{
    return dat->subgroups.size();
}

void BreakID::print_subgroups(std::ostream& out) {
    for (auto& grp : dat->subgroups) {
        out
        << "group size: " << grp->getSize()
        << " support: " << grp->getSupportSize() << endl;

        grp->print(out);
    }
}

void BreakID::break_symm()
{
    dat->brkr = new Breaker(dat->theory, dat->conf);
    for (auto& grp : dat->subgroups) {

        //Try to find matrix row interch. symmetries
        if (grp->getSize() > 1 && dat->conf->useMatrixDetection) {
            if (dat->conf->verbosity > 0) {
                cout << "Detecting row interchangeability..." << endl;
            }

            // Find set of clauses group permutates
            // add the subgroup to "grp->theory"
            dat->theory->setSubTheory(grp);

            // Upate group with matrix symmetries: matrixes, permutations
            grp->addMatrices();

            //Update stats
            dat->totalNbMatrices += grp->getNbMatrices();
            dat->totalNbRowSwaps += grp->getNbRowSwaps();
        }

        //Symmetry
        if (dat->conf->symBreakingFormLength > -1) {
            if (dat->conf->verbosity > 0) {
                cout << "*** Constructing symmetry breaking formula..." << endl;
            }
            grp->addBreakingClausesTo(*dat->brkr);
        }
    }
}

void BreakID::print_symm_break_stats()
{
    cout << "**** matrices detected: " << dat->totalNbMatrices << endl;
    cout << "**** row swaps detected: " << dat->totalNbRowSwaps << endl;

    cout << "**** extra binary symmetry breaking clauses added: "
              << dat->brkr->getNbBinClauses() << "\n";

    cout << "**** regular symmetry breaking clauses added: "
              << dat->brkr->getNbRegClauses() << "\n";

    cout << "**** row interchangeability breaking clauses added: "
              << dat->brkr->getNbRowClauses() << "\n";

    cout << "**** total symmetry breaking clauses added: "
              << dat->brkr->getAddedNbClauses() << "\n";

    cout << "**** auxiliary variables introduced: "
              << dat->brkr->getAuxiliaryNbVars() << "\n";
}

uint32_t BreakID::get_num_break_cls()
{
    return dat->brkr->getAddedNbClauses();
}

uint32_t BreakID::get_num_aux_vars()
{
    return dat->brkr->getAuxiliaryNbVars();
}

vector<vector<BID::BLit>> BreakID::get_brk_cls()
{
    return dat->brkr->get_brk_cls();
}

void BreakID::print_perms_and_matrices(std::ostream& out)
{
    for (auto grp : dat->subgroups) {
        grp->print(out);
    }
}

void BreakID::print_generators(std::ostream& out) {
    dat->theory->group->print(cout);
}

void BreakID::get_perms(vector<std::unordered_map<BLit, BLit> >* out)
{
    assert(out->empty());
    for (auto grp : dat->subgroups) {
        grp->get_perms_to(*out);
    }
}

std::string BreakID::get_sha1_version() const
{
    return BID::get_version_sha1();
}
