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
#include "bliss/graph.hh"


void Graph::initializeGraph(uint nbNodes, uint nbEdges, std::map<uint,uint>& lit2color,  std::vector<std::vector<uint> >& neighbours){
    bliss_g = new bliss::Graph(nbNodes);
    for(size_t n = 0; n < nbNodes; n++){
        bliss_g->change_color(n,lit2color[n]);
    }
    for(size_t n = 0; n < nbNodes; n++){
        for(auto other: neighbours[n]){
            bliss_g->add_edge(n,other);
        }
    }

};

void Graph::freeGraph() {
    delete(bliss_g);
}


uint Graph::getNbNodesFromGraph(){
    return bliss_g->get_nof_vertices();
}

uint Graph::getNbEdgesFromGraph() {
    return 0; //Not supported, not important
}

uint Graph::getColorOf(uint node) {
    return 0; //Not supported, only used for printing
}

uint Graph::nbNeighbours(uint node){
    return 0; //Not supported, only used for printing
}
uint Graph::getNeighbour(uint node, uint nbthNeighbour){
    return 0; //Not supported, only used for printing
}
void Graph::setNodeToNewColor(uint node) {
    bliss_g->change_color(node,colorcount.size());
}
std::vector<sptr<Permutation> > perms;

// This method is given to BLISS as a polymorphic consumer of the detected generator permutations

static void addBlissPermutation(void* param, const unsigned int n, const unsigned int* aut) {
    //TODO: currently, this cannot take the timeouts into account!

    sptr<Permutation> permu = std::make_shared<Permutation>();
    for (int i = 0; i < n; ++i) {
        if (i != aut[i]) {
            permu->addFromTo(i, aut[i]);
        }
    }
    permu->addPrimeSplitToVector(perms);

}

void Graph::getSymmetryGeneratorsInternal(std::vector<sptr<Permutation> >& out_perms){

    bliss::Stats stats;
    //bliss_g->set_splitting_heuristic(bliss::Graph::SplittingHeuristic::shs_fl); //TODO: to decide

    bliss_g->find_automorphisms(stats, &addBlissPermutation, stdout);

    std::swap(out_perms, perms);
}

Graph::Graph(std::unordered_set<sptr<Clause>, UVecHash, UvecEqual>& clauses) {
  //TODO: Why are we making undirected graphs? Efficiency? In principle, a lot could be directed (e.g., only edge from clause to lit -> more in LP)

  int n = 2 * nVars + clauses.size();

    std::map<uint,uint> lit2color{};

  // Initialize colors:
  for (uint i = 0; i < 2 * nVars; ++i) {
      lit2color[i] = 0;
  }
  colorcount.push_back(2 * nVars);

  for (int i = 2 * nVars; i < n; ++i) {
      lit2color[i] = 1;
  }
  colorcount.push_back(clauses.size());

    uint nbedges = 0;

  // Initialize edge lists
  // First construct for each node the list of neighbors
  std::vector<std::vector<uint> > neighbours(n);
  // Literals have their negations as neighbors
  for (uint l = 1; l <= nVars; ++l) {
    uint posID = encode(l);
    uint negID = encode(-l);
    neighbours[posID].push_back(negID);
    neighbours[negID].push_back(posID);
      nbedges += 2;
  }
  // Clauses have as neighbors the literals occurring in them
  uint c = 2 * nVars;
  for (auto cl : clauses) {
    for (auto lit : cl->lits) {
      neighbours[lit].push_back(c);
      neighbours[c].push_back(lit);
        nbedges += 2;
    }
    ++c;
  }


  //Now, initialize the internal graph
  initializeGraph(n,nbedges,lit2color,neighbours);




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

    std::map<uint,uint> lit2color{};
  uint current_node_index = 0;
  for (; current_node_index < 2 * nVars; ++current_node_index) {
    //Positive lits are colored with color0
      lit2color[current_node_index] = 0;
    ++current_node_index;
    //Negative lits are colored with color0
      lit2color[current_node_index] = 1;
  }
  colorcount.push_back(nVars);
  colorcount.push_back(nVars);

  //initialise the 4 extra nodes
  for(auto i: {2,3,4,5}) {
      lit2color[current_node_index] = i;
    current_node_index++;
    colorcount.push_back(1);
  }

  //Initialise colorcount!!!
  colorcount.reserve(maxcolor);
  for(auto col = 6; col < maxcolor; col ++ ) {
    colorcount.push_back(0);
  }

    uint nbedges = 0 ;
  // Initialize edge lists
  // First construct for each node the list of neighbors
  std::vector<std::vector<uint> > neighbours(n);
  // Literals have their negations as neighbors
  for (uint l = 1; l <= nVars; ++l) {
    uint posID = encode(l);
    uint negID = encode(-l);
    neighbours[posID].push_back(negID);
    neighbours[negID].push_back(posID);
      nbedges +=2;
  }

  for(auto r:rules) {
    auto bodyNode = current_node_index;
    current_node_index++;
    auto headNode = current_node_index;
    current_node_index++;

      lit2color[headNode] = 2;
    colorcount[2]++;

    if(r->ruleType == 1) {
        lit2color[bodyNode] = 3;
    }
    else if(r->ruleType == 3) {
        lit2color[bodyNode] = 4;
    }
    else if(r->ruleType == 2 || r->ruleType == 5) {
      auto color = boundToColor[r->bound];
        lit2color[bodyNode] = color;
    }
    else if(r->ruleType == 6) {
        lit2color[bodyNode] = 6;
    }

    colorcount[lit2color[bodyNode]]++;

    neighbours[bodyNode].push_back(headNode);
    neighbours[headNode].push_back(bodyNode);
      nbedges +=2;

    for(auto lit: r->headLits) {
      neighbours[lit].push_back(headNode);
      neighbours[headNode].push_back(lit);
        nbedges +=2;
    }
    if(r->ruleType != 5 && r->ruleType != 6) {
      for(auto lit: r->bodyLits) {
        neighbours[lit].push_back(bodyNode);
        neighbours[bodyNode].push_back(lit);
          nbedges +=2;
      }
    } else {
      for(u_int index = 0; index < r->bodyLits.size(); index++) {
        auto lit = r->bodyLits[index];
        auto weight = r->weights[index];
        auto extranode = current_node_index;
        current_node_index++;
          lit2color[extranode] = boundToColor[weight];
        neighbours[extranode].push_back(bodyNode);
        neighbours[bodyNode].push_back(extranode);
        neighbours[extranode].push_back(lit);
        neighbours[lit].push_back(extranode);
          nbedges +=4;
      }
    }

  }

  initializeGraph(n,nbedges,lit2color, neighbours);

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
  freeGraph();
}

void Graph::print() {
  for (int i = 0; i < getNbNodes(); ++i) {
    fprintf(stderr, "node %i with color %i has neighbours\n", i, getColorOf(i));
    for (int j = 0; j < nbNeighbours(i); ++j) {
      fprintf(stderr, "%i ", getNeighbour(i,j));
    }
    fprintf(stderr, "\n");
  }
}

uint Graph::getNbNodes(){
        return getNbNodesFromGraph();
}

uint Graph::getNbEdges() {
  return getNbEdgesFromGraph();
}

void Graph::setUniqueColor(uint lit) {
    auto color = getColorOf(lit);
  uint currentcount = colorcount[getColorOf(lit)];
  if (currentcount == 1) {
    return; // color was already unique
  }
  colorcount[color] = currentcount - 1;
  setNodeToNewColor(lit);
  colorcount.push_back(1);
}

void Graph::setUniqueColor(const std::vector<uint>& lits) {
  for (auto lit : lits) {
    setUniqueColor(lit);
  }
}

void Graph::getSymmetryGenerators(std::vector<sptr<Permutation> >& out_perms) {
    out_perms.clear();
    if (timeLimitPassed()) { // do not call saucy again when interrupted by time limit previously
        return;
    }

    if (verbosity > 1) {
        std::clog << "Searching graph automorphisms with time limit: " << timeLeft() << std::endl;
    }

    getSymmetryGeneratorsInternal(out_perms);
}
