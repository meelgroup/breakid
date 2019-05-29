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
    shared_ptr<Specification> theory;
    vector<shared_ptr<Group> > subgroups;
    uint32_t totalNbMatrices = 0;
    uint32_t totalNbRowSwaps = 0;
    Breaker* brkr = NULL;
    Config* conf = NULL;
};

BreakID::~BreakID()
{
    delete dat->conf;
    delete dat->brkr;
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

void BreakID::conf_timeLim(int64_t val)
{
    dat->conf->timeLim = val;
}

void BreakID::read_cnf(string filename_) {
    dat->theory = make_shared<CNF>(filename_, dat->conf);
}

void BreakID::start_dynamic_cnf(uint32_t nVars)
{
    dat->theory = make_shared<OnlCNF>(nVars, dat->conf);
}

void BreakID::add_clause(BID::BLit* start, size_t num)
{
    dat->theory->add_clause(start, num);
}

void BreakID::end_dynamic_cnf()
{
    dat->theory->end_dynamic_cnf();
}

void BreakID::print_graph() {
    dat->theory->getGraph()->print();
}

uint32_t BreakID::get_num_generators()
{
    return dat->theory->getGroup()->getSize();
}

void BreakID::print_generators() {
    dat->theory->getGroup()->print(cout);
}

void BreakID::detect_subgroups() {
    dat->theory->getGroup()->getDisjointGenerators(dat->subgroups);
}

void BreakID::print_subgroups() {
    cout
    << "**** subgroups detected: "
    << dat->subgroups.size() << endl;

    for (auto grp : dat->subgroups) {
        cout
        << "group size: " << grp->getSize()
        << " support: " << grp->getSupportSize() << endl;

        grp->print(cout);
    }
}

void BreakID::clean_theory() {
    // improve some memory overhead
    dat->theory->cleanUp();
}

void BreakID::break_symm()
{
    dat->brkr = new Breaker(dat->theory, dat->conf);
    for (auto grp : dat->subgroups) {
        //cout << "NOTE: Matrix detection disabled as current code is too slow" << endl;
        if (grp->getSize() > 1 && dat->conf->useMatrixDetection) {
            if (dat->conf->verbosity > 1) {
                cout << "*** Detecting row interchangeability..." << endl;
            }
            dat->theory->setSubTheory(grp);
            grp->addMatrices();
            dat->totalNbMatrices += grp->getNbMatrices();
            dat->totalNbRowSwaps += grp->getNbRowSwaps();
        }
        if (dat->conf->symBreakingFormLength > -1) {
            if (dat->conf->verbosity > 1) {
                cout << "*** Constructing symmetry breaking formula..." << endl;
            }
            grp->addBreakingClausesTo(*dat->brkr);
        } // else no symmetry breaking formulas are needed :)
        grp.reset();
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

void BreakID::write_final_cnf(bool only_breakers) {
    dat->brkr->print(only_breakers);
}

void BreakID::print_generators(string symFile)
{
    ofstream fp_out;
    fp_out.open(symFile, std::ios::out);
    for (auto grp : dat->subgroups) {
        grp->print(fp_out);
    }
    fp_out.close();
}

std::string BreakID::get_sha1_version() const
{
    return BID::get_version_sha1();
}
