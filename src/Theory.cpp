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

void CNF::readCNF(string& filename)
{
    if (conf->verbosity > 0) {
        cout << "*** Reading CNF: " << filename << endl;
    }

    ifstream file(filename);
    if (!file) {
        std::cout << "ERROR: No CNF file found." << endl;
        exit(-1);
    }
    string line;
    set<uint32_t> inclause = set<uint32_t>();
    while (getline(file, line)) {
        if (line.size() == 0 || line.front() == 'c') {
            // do nothing, this is a comment line
        } else if (line.front() == 'p') {
            string info = line.substr(6);
            istringstream iss(info);
            uint32_t nbVars;
            iss >> nbVars;
            uint32_t nbClauses;
            iss >> nbClauses;
            if (conf->verbosity > 1) {
                cout << "CNF header stated " << nbVars << " vars and "
                          << nbClauses << " clauses" << endl;
            }
            conf->nVars = nbVars;
            clauses.reserve(nbClauses);
        } else {
            //  parse the clauses, removing tautologies and double lits
            istringstream iss(line);
            int l;
            while (iss >> l) {
                if (l == 0) {
                    if (inclause.size() == 0) {
                        std::cout << "ERROR: Theory can not contain empty clause." << endl;
                        exit(-1);
                    }
                    bool isTautology = false;
                    for (auto lit : inclause) {
                        if (inclause.count(neg(lit)) > 0) {
                            isTautology = true;
                            break;
                        }
                    }
                    if (!isTautology) {
                        shared_ptr<Clause> cl(new Clause(inclause));
                        clauses.insert(cl);
                    }
                    inclause.clear();
                } else {
                    if ((uint32_t)abs(l) > conf->nVars) {
                        conf->nVars = abs(l);
                    }
                    inclause.insert(encode(l));
                }
            }
        }
    }
}

CNF::CNF(string& filename, Config* _conf) :
    conf(_conf)
{
    readCNF(filename);
    if (conf->verbosity > 0) {
        cout << "*** Creating first graph..." << endl;
    }
    graph = make_shared<Graph>(clauses, conf);
    if (conf->verbosity > 1) {
        cout << "**** Number of nodes: " << graph->getNbNodes()
                  << endl;
        cout << "**** Number of edges: " << graph->getNbEdges()
                  << endl;
    }

    group = make_shared<Group>(conf);
    if (conf->verbosity > 0) {
        cout << "*** Detecting symmetry group..." << endl;
    }
    vector<shared_ptr<Permutation> > symgens;
    graph->getSymmetryGenerators(symgens);
    for (auto symgen : symgens) {
        group->add(symgen);
    }
}

CNF::CNF(vector<shared_ptr<Clause> >& clss, shared_ptr<Group> grp)
{
    clauses.insert(clss.cbegin(), clss.cend());
    graph = make_shared<Graph>(clauses, conf);
    group = grp;
    for (uint32_t l = 0; l < 2 * conf->nVars; ++l) {
        if (!grp->permutes(l)) {
            graph->setUniqueColor(l);
        }
    }
    for (uint32_t m = 0; m < grp->getNbMatrices(); ++m) {
        auto mat = grp->getMatrix(m);
        for (uint32_t r = 0; r < mat->nbRows() - 1; ++r) {
            getGraph()->setUniqueColor(*(mat->getRow(r)));
        }
    }
}

CNF::~CNF()
{
}

void CNF::print(std::ostream& out)
{
    for (auto clause : clauses) {
        clause->print(out);
    }
}

uint32_t CNF::getSize()
{
    return clauses.size();
}

void CNF::setSubTheory(shared_ptr<Group> subgroup)
{
    //TODO: what is this method supposed to do: keep all clauses that are not mapped to themselves?
    //TODO: Is it simply made approximative on purpose or by accident?
    vector<shared_ptr<Clause> > subclauses;
    for (auto cl : clauses) {
        for (auto lit : cl->lits) {
            if (subgroup->permutes(lit)) {
                subclauses.push_back(cl);
                break;
            }
        }
    }
    subgroup->theory = new CNF(subclauses, subgroup);
}

bool CNF::isSymmetry(Permutation& prm)
{
    for (auto cl : clauses) {
        shared_ptr<Clause> symmetrical(new Clause());
        if (!prm.getImage(cl->lits, symmetrical->lits)) {
            continue;
        }
        std::sort(symmetrical->lits.begin(), symmetrical->lits.end());
        if (clauses.count(symmetrical) == 0) {
            return false;
        }
    }
    return true;
}

//// ONLINE

OnlCNF::OnlCNF(uint32_t nVars, uint32_t _num_cls, Config* _conf) :
    conf(_conf)
{
    conf->nVars = nVars;
    num_cls = _num_cls;
    graph = make_shared<Graph>(num_cls, conf);
}

OnlCNF::~OnlCNF()
{
}

void OnlCNF::add_clause(BID::BLit* lits, uint32_t size)
{
    graph->add_clause(lits, size);
}

void OnlCNF::end_dynamic_cnf()
{
    graph->end_dynamic_cnf();

    if (conf->verbosity > 1) {
        cout << "**** Number of nodes: " << graph->getNbNodes()
                  << endl;
        cout << "**** Number of edges: " << graph->getNbEdges()
                  << endl;
    }

    group = make_shared<Group>(conf);
    if (conf->verbosity > 0) {
        cout << "*** Detecting symmetry group..." << endl;
    }
    vector<shared_ptr<Permutation> > symgens;
    graph->getSymmetryGenerators(symgens);
    for (auto symgen : symgens) {
        group->add(symgen);
    }
}

void OnlCNF::print(std::ostream&)
{
    assert(false);
}

uint32_t OnlCNF::getSize()
{
    return num_cls;
}

void OnlCNF::setSubTheory(shared_ptr<Group>)
{
    //this will ONLY kill matrix detection
    //disabled matrix detection instead.
    assert(false && "Not implemented");
}

bool OnlCNF::isSymmetry(Permutation&)
{
    //this will ONLY kill matrix detection
    //disabled matrix detection instead.
    assert(false && "Not implemented");
    return true;
}


/******************
 * SPECIFICATION
 *
 */

Specification::~Specification()
{
}

Specification::Specification()
{
}

shared_ptr<Graph> Specification::getGraph()
{
    return graph;
}

shared_ptr<Group> Specification::getGroup()
{
    return group;
}

void Specification::cleanUp()
{
    graph.reset();
    group.reset();
}

/******************
 * LOGIC PROGRAM
 */

void checkVarExists(int lit, Config* conf)
{
    if ((uint32_t)abs(lit) > conf->nVars) {
        conf->nVars = abs(lit);
    }
}

LogicProgram::LogicProgram(vector<shared_ptr<Rule> >& rls, shared_ptr<Group> grp, Config* _conf) :
    conf(_conf)
{
    rules.insert(rls.cbegin(), rls.cend());
    graph = make_shared<Graph>(rules, conf);
    group = grp;
    for (uint32_t l = 0; l < 2 * conf->nVars; ++l) {
        if (!grp->permutes(l)) {
            graph->setUniqueColor(l);
        }
    }
    for (uint32_t m = 0; m < grp->getNbMatrices(); ++m) {
        auto mat = grp->getMatrix(m);
        for (uint32_t r = 0; r < mat->nbRows() - 1; ++r) {
            getGraph()->setUniqueColor(*(mat->getRow(r)));
        }
    }
}

//CNF(vector<shared_ptr<Clause> >& clss, shared_ptr<Group> grp);
LogicProgram::~LogicProgram()
{
}

void LogicProgram::print(std::ostream& out)
{
    for (auto rule : rules) {
        rule->print(out);
    }
}

uint32_t LogicProgram::getSize()
{
    return rules.size();
}

void LogicProgram::setSubTheory(shared_ptr<Group> subgroup)
{
    //TODO: what is this method supposed to do: keep all clauses that are not mapped to themselves? Is it simply made approximative on purpose or by accident?
    vector<shared_ptr<Rule> > subrules;
    for (auto r : rules) {
        bool found = false;
        for (auto lit : r->headLits) {
            if (found) {
                break;
            }
            if (subgroup->permutes(lit)) {
                subrules.push_back(r);
                found = true;
            }
        }
        for (auto lit : r->bodyLits) {
            if (found) {
                break;
            }
            if (subgroup->permutes(lit)) {
                subrules.push_back(r);
                found = true;
            }
        }
    }
    subgroup->theory = new LogicProgram(subrules, subgroup, conf);
}

bool LogicProgram::isSymmetry(Permutation& prm)
{
    for (auto r : rules) {
        shared_ptr<Rule> symmetrical(new Rule());
        if (!(prm.getImage(r->headLits, symmetrical->headLits) |
              prm.getImage(r->bodyLits, symmetrical->bodyLits))) {
            continue;
        }
        symmetrical->weights = r->weights;
        symmetrical->ruleType = r->ruleType;
        symmetrical->bound = r->bound;
        std::sort(symmetrical->headLits.begin(), symmetrical->headLits.end());
        if (r->ruleType != 5 && r->ruleType != 6) {
            std::sort(symmetrical->bodyLits.begin(),
                      symmetrical->bodyLits.end());
        }
        if (rules.count(symmetrical) == 0) {
            return false;
        }
    }
    return true;
}
