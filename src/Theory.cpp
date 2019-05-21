#include "Theory.hpp"
#include <fstream>
#include <sstream>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <bitset>
#include "Graph.hpp"
#include "Algebraic.hpp"
#include "Breaking.hpp"

//=========================CNF==================================================

using namespace std;

void CNF::readCNF(std::string& filename) {
  if (verbosity > 0) {
    std::clog << "*** Reading CNF: " << filename << std::endl;
  }

  ifstream file(filename);
  if (!file) {
    gracefulError("No CNF file found.");
  }
  string line;
  set<uint> inclause = set<uint>();
  while (getline(file, line)) {
    if (line.size() == 0 || line.front() == 'c') {
      // do nothing, this is a comment line
    } else if (line.front() == 'p') {
      string info = line.substr(6);
      istringstream iss(info);
      uint nbVars;
      iss >> nbVars;
      uint nbClauses;
      iss >> nbClauses;
      if (verbosity > 1) {
        std::clog << "CNF header stated " << nbVars << " vars and " << nbClauses << " clauses" << std::endl;
      }
      nVars = nbVars;
      clauses.reserve(nbClauses);
    } else {
      //  parse the clauses, removing tautologies and double lits
      istringstream iss(line);
      int l;
      while (iss >> l) {
        if (l == 0) {
          if (inclause.size() == 0) {
            gracefulError("Theory can not contain empty clause.");
          }
          bool isTautology = false;
          for (auto lit : inclause) {
            if (inclause.count(neg(lit)) > 0) {
              isTautology = true;
              break;
            }
          }
          if (not isTautology) {
            sptr<Clause> cl(new Clause(inclause));
            clauses.insert(cl);
          }
          inclause.clear();
        } else {
          if ((uint) abs(l) > nVars) {
            nVars = abs(l);
          }
          inclause.insert(encode(l));
        }
      }
    }
  }
}

CNF::CNF(std::string& filename) {
  readCNF(filename);
  if (verbosity > 0) {
    std::clog << "*** Creating first graph..." << std::endl;
  }
  graph = make_shared<Graph>(clauses);
  if(verbosity > 1){
    std::clog << "**** Number of nodes: " << graph->getNbNodes() << std::endl;
    std::clog << "**** Number of edges: " << graph->getNbEdges() << std::endl;
  }

  group = make_shared<Group>();
  if (verbosity > 0) {
    std::clog << "*** Detecting symmetry group..." << std::endl;
  }
  std::vector<sptr<Permutation> > symgens;
  graph->getSymmetryGenerators(symgens);
  for (auto symgen : symgens) {
    group->add(symgen);
  }
}

CNF::CNF(std::vector<sptr<Clause> >& clss, sptr<Group> grp) {
  clauses.insert(clss.cbegin(), clss.cend());
  graph = make_shared<Graph>(clauses);
  group = grp;
  for (uint l = 0; l < 2 * nVars; ++l) {
    if (not grp->permutes(l)) {
      graph->setUniqueColor(l);
    }
  }
  for(uint m=0; m<grp->getNbMatrices(); ++m){
    auto mat = grp->getMatrix(m);
    for(uint r=0; r<mat->nbRows()-1; ++r){
      getGraph()->setUniqueColor(*(mat->getRow(r)));
    }
  }
}

CNF::~CNF() {
}

void CNF::print(std::ostream& out) {
  for (auto clause : clauses) {
    clause->print(out);
  }
}

uint CNF::getSize() {
  return clauses.size();
}

void CNF::setSubTheory(sptr<Group> subgroup) {
  //TODO: what is this method supposed to do: keep all clauses that are not mapped to themselves? Is it simply made approximative on purpose or by accident?
  std::vector<sptr<Clause> > subclauses;
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

bool CNF::isSymmetry(Permutation& prm) {
  for (auto cl : clauses) {
    sptr<Clause> symmetrical(new Clause());
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

/******************
 * SPECIFICATION
 *
 */

Specification::~Specification() {

}

Specification::Specification() {

}

sptr<Graph> Specification::getGraph() {
  return graph;
}

sptr<Group> Specification::getGroup() {
  return group;
}

void Specification::cleanUp() {
  graph.reset();
  group.reset();
}

/******************
 * LOGIC PROGRAM
 */

void checkVarExists(int lit) {
  if ((uint) abs(lit) > nVars) {
    nVars = abs(lit);
  }
}

LogicProgram::LogicProgram(std::vector<sptr<Rule> >& rls, sptr<Group> grp) {
  rules.insert(rls.cbegin(), rls.cend());
  graph = make_shared<Graph>(rules);
  group = grp;
  for (uint l = 0; l < 2 * nVars; ++l) {
    if (not grp->permutes(l)) {
      graph->setUniqueColor(l);
    }
  }
  for(uint m=0; m<grp->getNbMatrices(); ++m){
    auto mat = grp->getMatrix(m);
    for(uint r=0; r<mat->nbRows()-1; ++r){
      getGraph()->setUniqueColor(*(mat->getRow(r)));
    }
  }
}

//CNF(std::vector<sptr<Clause> >& clss, sptr<Group> grp);
LogicProgram::~LogicProgram() {

}

void LogicProgram::print(std::ostream& out) {
  for (auto rule : rules) {
    rule->print(out);
  }
}

uint LogicProgram::getSize() {
  return rules.size();

}

void LogicProgram::setSubTheory(sptr<Group> subgroup) {
  //TODO: what is this method supposed to do: keep all clauses that are not mapped to themselves? Is it simply made approximative on purpose or by accident?
  std::vector<sptr<Rule> > subrules;
  for (auto r : rules) {
    bool found = false;
    for (auto lit : r->headLits) {
      if(found) {
        break;
      }
      if (subgroup->permutes(lit)) {
        subrules.push_back(r);
        found = true;
      }
    }
    for (auto lit : r->bodyLits) {
      if(found) {
        break;
      }
      if (subgroup->permutes(lit)) {
        subrules.push_back(r);
        found = true;
      }
    }
  }
  subgroup->theory = new LogicProgram(subrules, subgroup);
}

bool LogicProgram::isSymmetry(Permutation &prm) {
  for (auto r : rules) {
    sptr<Rule> symmetrical(new Rule());
    if ( ! (prm.getImage(r->headLits, symmetrical->headLits) | prm.getImage(r->bodyLits, symmetrical->bodyLits) )) {
      continue;
    }
    symmetrical->weights = r->weights;
    symmetrical->ruleType = r->ruleType;
    symmetrical->bound = r->bound;
    std::sort(symmetrical->headLits.begin(), symmetrical->headLits.end());
    if(r->ruleType != 5 && r->ruleType != 6) {
      std::sort(symmetrical->bodyLits.begin(), symmetrical->bodyLits.end());
    }
    if (rules.count(symmetrical) == 0) {
      return false;
    }
  }
  return true;
}
