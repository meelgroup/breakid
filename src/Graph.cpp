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

#include "Graph.hpp"
#include "saucy.h"

//=====================SAUCYWRAPPER=METHODS=====================================

std::vector<sptr<Permutation> > perms;

// This method is given to Saucy as a polymorphic consumer of the detected generator permutations

static int addPermutation(int n, const int *ct_perm, int nsupp, int *support, void *arg) {
  if (n == 0 || nsupp == 0) {
    return not timeLimitPassed();
  }

  sptr<Permutation> perm = std::make_shared<Permutation>();
  for (int i = 0; i < n; ++i) {
    if (i != ct_perm[i]) {
      perm->addFromTo(i, ct_perm[i]);
    }
  }
  perms.push_back(perm);

  return not timeLimitPassed();
}

//=====================GRAPH====================================================

Graph::Graph(std::unordered_set<sptr<Clause>, UVecHash, UvecEqual>& clauses) {
  //TODO: Why are we making undirected graphs? Efficiency? In principle, a lot could be directed (e.g., only edge from clause to lit -> more in LP)
  sg = (saucy_graph*) malloc(sizeof (struct saucy_graph));

  int n = 2 * nVars + clauses.size();

  // Initialize colors:
  sg->colors = (int*) malloc(n * sizeof (int));
  for (uint i = 0; i < 2 * nVars; ++i) {
    sg->colors[i] = 0;
  }
  colorcount.push_back(2 * nVars);

  for (int i = 2 * nVars; i < n; ++i) {
    sg->colors[i] = 1;
  }
  colorcount.push_back(clauses.size());

  // Initialize edge lists
  // First construct for each node the list of neighbors
  std::vector<std::vector<uint> > neighbours(n);
  // Literals have their negations as neighbors
  for (uint l = 1; l <= nVars; ++l) {
    uint posID = encode(l);
    uint negID = encode(-l);
    neighbours[posID].push_back(negID);
    neighbours[negID].push_back(posID);
  }
  // Clauses have as neighbors the literals occurring in them
  uint c = 2 * nVars;
  for (auto cl : clauses) {
    for (auto lit : cl->lits) {
      neighbours[lit].push_back(c);
      neighbours[c].push_back(lit);
    }
    ++c;
  }

  // now count the number of neighboring nodes
  sg->adj = (int*) malloc((n + 1) * sizeof (int));
  sg->adj[0] = 0;
  int ctr = 0;
  for (auto nblist : neighbours) {
    sg->adj[ctr + 1] = sg->adj[ctr] + nblist.size();
    ++ctr;
  }

  // finally, initialize the lists of neighboring nodes, C-style
  sg->edg = (int*) malloc(sg->adj[n] * sizeof (int));
  ctr = 0;
  for (auto nblist : neighbours) {
    for (auto l : nblist) {
      sg->edg[ctr] = l;
      ++ctr;
    }
  }

  sg->n = n;
  sg->e = sg->adj[n] / 2;

  // look for unused lits, make their color unique so that no symmetries on them are found
  // useful for subgroups
  std::unordered_set<uint> usedLits;
  for (auto cl : clauses) {
    for (auto lit : cl->lits) {
      usedLits.insert(lit);
      usedLits.insert(neg(lit));
    }
  }
  for(uint i=0; i<2*nVars; ++i) {
    if(usedLits.count(i)==0) {
      setUniqueColor(i);
    }
  }

  // fix fixedLits
  setUniqueColor(fixedLits);
}
Graph::Graph(std::unordered_set<sptr<Rule>, UVecHash, UvecEqual>& rules) {
  sg = (saucy_graph*) malloc(sizeof (struct saucy_graph));

  //We create this graph:
  // color0 for positive literals
  // color1 for negative literals
  // color2 for head nodes
  // color3 for body nodes of rule-type 1 or 8 or integrity constraints
  // color4 for body nodes of rule-type 3
  // color5 for body nodes of rule type 6
  // colorn for body nodes of rule-type 2 or 5 with bound, where n is obtained from bound through the boundToColor map below.
  // arrows between positive and negative literals of the same atoms
  // arrows from head to literals occurring in it (and back)
  // arrows from body to literals occurring in it (posbodylits and negbodylits can be treated as one large list; separation is only for printing purposes) (and back)
  // arrows from head to body (and back)

  //first we collect all the bound
  auto maxcolor = 6;
  std::unordered_map<int,int> boundToColor;
  int nbextralits = 0;

  for(auto r: rules) {
    if(r->ruleType== 2 || r->ruleType == 5) {
      auto bound = r->bound;
      auto& num = boundToColor[bound];
      if(not num) {
        num = maxcolor;
        maxcolor ++;
      }
    }
    if(r->ruleType == 5 || r->ruleType == 6) {
      nbextralits += r->weights.size();
      for(auto w: r->weights) {
        auto& num = boundToColor[w];
        if(not num) {
          num = maxcolor;
          maxcolor ++;
        }
      }
    }
  }

  //Number of nodes: 2 for each variable + two for each rule (head+body)
  //The three is to make sure that all colors are used (cfr colorcount documentation)
  int n = 2 * nVars + 2 * rules.size() + 4 + nbextralits;

  // Initialize colors: take enough space
  sg->colors = (int*) malloc(n * sizeof (int));
  uint current_node_index = 0;
  for (; current_node_index < 2 * nVars; ++current_node_index) {
    //Positive lits are colored with color0
    sg->colors[current_node_index] = 0;
    ++current_node_index;
    //Negative lits are colored with color0
    sg->colors[current_node_index] = 1;
  }
  colorcount.push_back(nVars);
  colorcount.push_back(nVars);

  //initialise the 4 extra nodes
  for(auto i: {2,3,4,5}) {
    sg->colors[current_node_index] = i;
    current_node_index++;
    colorcount.push_back(1);
  }

  //Initialise colorcount!!!
  colorcount.reserve(maxcolor);
  for(auto col = 6; col < maxcolor; col ++ ) {
    colorcount.push_back(0);
  }

  // Initialize edge lists
  // First construct for each node the list of neighbors
  std::vector<std::vector<uint> > neighbours(n);
  // Literals have their negations as neighbors
  for (uint l = 1; l <= nVars; ++l) {
    uint posID = encode(l);
    uint negID = encode(-l);
    neighbours[posID].push_back(negID);
    neighbours[negID].push_back(posID);
  }

  for(auto r:rules) {
    auto bodyNode = current_node_index;
    current_node_index++;
    auto headNode = current_node_index;
    current_node_index++;

    sg->colors[headNode] = 2;
    colorcount[2]++;

    if(r->ruleType == 1) {
      sg->colors[bodyNode] = 3;
    }
    else if(r->ruleType == 3) {
      sg->colors[bodyNode] = 4;
    }
    else if(r->ruleType == 2 || r->ruleType == 5) {
      auto color = boundToColor[r->bound];
      sg->colors[bodyNode] = color;
    }
    else if(r->ruleType == 6) {
      sg->colors[bodyNode] = 6;
    }

    colorcount[sg->colors[bodyNode]]++;

    neighbours[bodyNode].push_back(headNode);
    neighbours[headNode].push_back(bodyNode);

    for(auto lit: r->headLits) {
      neighbours[lit].push_back(headNode);
      neighbours[headNode].push_back(lit);
    }
    if(r->ruleType != 5 && r->ruleType != 6) {
      for(auto lit: r->bodyLits) {
        neighbours[lit].push_back(bodyNode);
        neighbours[bodyNode].push_back(lit);
      }
    } else {
      for(u_int index = 0; index < r->bodyLits.size(); index++) {
        auto lit = r->bodyLits[index];
        auto weight = r->weights[index];
        auto extranode = current_node_index;
        current_node_index++;
        sg->colors[extranode] = boundToColor[weight];
        neighbours[extranode].push_back(bodyNode);
        neighbours[bodyNode].push_back(extranode);
        neighbours[extranode].push_back(lit);
        neighbours[lit].push_back(extranode);
      }
    }

  }

  // now count the number of neighboring nodes
  sg->adj = (int*) malloc((n + 1) * sizeof (int));
  sg->adj[0] = 0;
  int ctr = 0;
  for (auto nblist : neighbours) {
    sg->adj[ctr + 1] = sg->adj[ctr] + nblist.size();
    ++ctr;
  }

// finally, initialize the lists of neighboring nodes, C-style
  sg->edg = (int*) malloc(sg->adj[n] * sizeof (int));
  ctr = 0;
  for (auto nblist : neighbours) {
    for (auto l : nblist) {
      sg->edg[ctr] = l;
      ++ctr;
    }
  }

  sg->n = n;
  sg->e = sg->adj[n] / 2;

// look for unused lits, make their color unique so that no symmetries on them are found
// useful for subgroups
  std::unordered_set<uint> usedLits;
  for (auto cl : rules) {
    for (auto lit : cl->headLits) {
      usedLits.insert(lit);
      usedLits.insert(neg(lit));
    }
    for (auto lit : cl->bodyLits) {
      usedLits.insert(lit);
      usedLits.insert(neg(lit));
    }
  }
  for(uint i=0; i<2*nVars; ++i) {
    if(usedLits.count(i)==0) {
      setUniqueColor(i);
    }
  }

// fix fixedLits
  setUniqueColor (fixedLits);
}

Graph::~Graph() {
  free(sg->adj);
  free(sg->edg);
  free(sg->colors);
  free(sg);
}

void Graph::print() {
  for (int i = 0; i < sg->n; ++i) {
    fprintf(stderr, "node %i with color %i has neighbours\n", i, sg->colors[i]);
    for (int j = sg->adj[i]; j < sg->adj[i + 1]; ++j) {
      fprintf(stderr, "%i ", sg->edg[j]);
    }
    fprintf(stderr, "\n");
  }
}

uint Graph::getNbNodes() {
  return (uint) sg->n;
}

uint Graph::getNbEdges() {
  return (uint) sg->e;
}

void Graph::setUniqueColor(uint lit) {
  uint currentcount = colorcount[sg->colors[lit]];
  if (currentcount == 1) {
    return; // color was already unique
  }
  colorcount[sg->colors[lit]] = currentcount - 1;
  sg->colors[lit] = colorcount.size() ;
  colorcount.push_back(1);
}

void Graph::setUniqueColor(const std::vector<uint>& lits) {
  for (auto lit : lits) {
    setUniqueColor(lit);
  }
}

void Graph::getSymmetryGenerators(std::vector<sptr<Permutation> >& out_perms) {
  out_perms.clear();

  if(getNbNodes()<=2*nVars) { // Saucy does not like empty graphs, so don't call Saucy with an empty graph
      return;
    }

    if (timeLimitPassed()) { // do not call saucy again when interrupted by time limit previously
      return;
    }
    if (verbosity > 1) {
      std::clog << "Running Saucy with time limit: " << timeLeft() << std::endl;
    }

    // WARNING: make sure that maximum color does not surpass the number of colors, as this seems to give saucy trouble...
    struct saucy* s = saucy_alloc(sg->n, timeLeft());
    struct saucy_stats stats;
    saucy_search(s, sg, 0, addPermutation, 0, &stats);
    saucy_free(s);
    // TODO: how to check whether sg is correctly freed?

    std::swap(out_perms,perms);
  }
