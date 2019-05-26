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

using std::cout;
using std::endl;

Graph::Graph(uint32_t nClauses, Config* _conf) :
    conf(_conf)
{
    uint32_t n = 2 * conf->nVars + nClauses;
    bliss_g = new bliss::Graph(n);
    used_lits.resize(conf->nVars*2, 0);
    color.resize(n);

    // Initialize colors
    for (uint32_t i = 0; i < 2 * conf->nVars; ++i) {
        bliss_g->change_color(i, 0);
        color[i] = 0;
    }
    colorcount.push_back(2 * conf->nVars);

    for (uint32_t i = 2 * conf->nVars; i < n; ++i) {
        bliss_g->change_color(i, 1);
        color[i] = 1;
    }
    colorcount.push_back(nClauses);

    // Initialize edge lists
    // First construct for each node the list of neighbors
    // Literals have their negations as neighbors
    for (uint32_t l = 1; l <= conf->nVars; ++l) {
        uint32_t posID = encode(l);
        uint32_t negID = encode(-l);
        bliss_g->add_edge(posID, negID);
        bliss_g->add_edge(negID, posID);
    }

    cur_cl_num = 2 * conf->nVars;
}

void Graph::add_clause(BID::BLit* lits, uint32_t size)
{
    if (size == 1) {
        BID::BLit lit = lits[0];
        setUniqueColor(encode(lit_to_weird(lit)));
        return;
    }

    // Clauses have as neighbors the literals occurring in them
    for(size_t i = 0; i < size; i++) {
        BID::BLit l = lits[i];
        bliss_g->add_edge(cur_cl_num, encode(lit_to_weird(l)));
        bliss_g->add_edge(encode(lit_to_weird(l)), cur_cl_num);
        used_lits[encode(lit_to_weird(l))] = 1;
    }
    cur_cl_num++;
}

void Graph::end_dynamic_cnf()
{
    // look for unused lits, make their color unique so that no symmetries on them are found
    // useful for subgroups
    for (uint32_t i = 0; i < 2 * conf->nVars; ++i) {
        if (used_lits[i] == 0) {
            setUniqueColor(i);
        }
    }

    // there must be NO fixed lits, this is ASSERTED
}

void Graph::initializeGraph(uint32_t nbNodes, uint32_t nbEdges,
                            std::map<uint32_t, uint32_t>& lit2color,
                            vector<vector<uint32_t> >& neighbours)
{
    bliss_g = new bliss::Graph(nbNodes);
    color.resize(nbNodes);

    for (size_t n = 0; n < nbNodes; n++) {
        bliss_g->change_color(n, lit2color[n]);
        color[n] = lit2color[n];
    }
    for (size_t n = 0; n < nbNodes; n++) {
        for (auto other : neighbours[n]) {
            bliss_g->add_edge(n, other);
        }
    }
}

void Graph::freeGraph()
{
    delete (bliss_g);
}

uint32_t Graph::getNbNodesFromGraph()
{
    return bliss_g->get_nof_vertices();
}

uint32_t Graph::getNbEdgesFromGraph()
{
    return 0; //Not supported, not important
}

uint32_t Graph::getColorOf(uint32_t node)
{
    assert(node < color.size());
    return color[node];
}

uint32_t Graph::nbNeighbours(uint32_t /*node*/)
{
    return 0; //Not supported, only used for printing
}
uint32_t Graph::getNeighbour(uint32_t /*node*/, uint32_t /*nbthNeighbour*/)
{
    return 0; //Not supported, only used for printing
}

// This method is given to BLISS as a polymorphic consumer of the detected generator permutations

static void addBlissPermutation(
    void* param, const unsigned int n,
    const unsigned int* aut)
{
    Graph* g = (Graph*)param;

    shared_ptr<Permutation> permu = std::make_shared<Permutation>(g->conf);
    for (unsigned i = 0; i < n; ++i) {
        if (i != aut[i]) {
            permu->addFromTo(i, aut[i]);
        }
    }
    permu->addPrimeSplitToVector(g->perms);
}

void Graph::getSymmetryGeneratorsInternal(
    vector<shared_ptr<Permutation> >& out_perms)
{
    bliss::Stats stats;
    //bliss_g->set_splitting_heuristic(bliss::Graph::SplittingHeuristic::shs_fl); //TODO: to decide

    bliss_g->find_automorphisms(stats, &addBlissPermutation, (void*)this);

    std::swap(out_perms, perms);
}

Graph::Graph(std::unordered_set<shared_ptr<Clause>, UVecHash, UvecEqual>& clauses, Config* _conf) :
    conf(_conf)
{
    //TODO: Why are we making undirected graphs? Efficiency? In principle, a lot could be directed (e.g., only edge from clause to lit -> more in LP)

    int n = 2 * conf->nVars + clauses.size();

    std::map<uint32_t, uint32_t> lit2color{};

    // Initialize colors:
    for (uint32_t i = 0; i < 2 * conf->nVars; ++i) {
        lit2color[i] = 0;
    }
    colorcount.push_back(2 * conf->nVars);

    for (int i = 2 * conf->nVars; i < n; ++i) {
        lit2color[i] = 1;
    }
    colorcount.push_back(clauses.size());

    uint32_t nbedges = 0;

    // Initialize edge lists
    // First construct for each node the list of neighbors
    vector<vector<uint32_t> > neighbours(n);
    // Literals have their negations as neighbors
    for (uint32_t l = 1; l <= conf->nVars; ++l) {
        uint32_t posID = encode(l);
        uint32_t negID = encode(-l);
        neighbours[posID].push_back(negID);
        neighbours[negID].push_back(posID);
        nbedges += 2;
    }
    // Clauses have as neighbors the literals occurring in them
    uint32_t c = 2 * conf->nVars;
    for (auto cl : clauses) {
        for (auto lit : cl->lits) {
            neighbours[lit].push_back(c);
            neighbours[c].push_back(lit);
            nbedges += 2;
        }
        ++c;
    }

    //Now, initialize the internal graph
    initializeGraph(n, nbedges, lit2color, neighbours);

    // look for unused lits, make their color unique so that no symmetries on them are found
    // useful for subgroups
    std::unordered_set<uint32_t> usedLits;
    for (auto cl : clauses) {
        for (auto lit : cl->lits) {
            usedLits.insert(lit);
            usedLits.insert(neg(lit));
        }
    }
    for (uint32_t i = 0; i < 2 * conf->nVars; ++i) {
        if (usedLits.count(i) == 0) {
            setUniqueColor(i);
        }
    }

    // fix conf->fixedLits
    setUniqueColor(conf->fixedLits);
}

Graph::Graph(std::unordered_set<shared_ptr<Rule>, UVecHash, UvecEqual>& rules, Config* _conf) :
    conf(_conf)
{
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
    std::unordered_map<int, int> boundToColor;
    int nbextralits = 0;

    for (auto r : rules) {
        if (r->ruleType == 2 || r->ruleType == 5) {
            auto bound = r->bound;
            auto& num = boundToColor[bound];
            if (!num) {
                num = maxcolor;
                maxcolor++;
            }
        }
        if (r->ruleType == 5 || r->ruleType == 6) {
            nbextralits += r->weights.size();
            for (auto w : r->weights) {
                auto& num = boundToColor[w];
                if (!num) {
                    num = maxcolor;
                    maxcolor++;
                }
            }
        }
    }

    //Number of nodes: 2 for each variable + two for each rule (head+body)
    //The three is to make sure that all colors are used (cfr colorcount documentation)
    int n = 2 * conf->nVars + 2 * rules.size() + 4 + nbextralits;

    std::map<uint32_t, uint32_t> lit2color{};
    uint32_t current_node_index = 0;
    for (; current_node_index < 2 * conf->nVars; ++current_node_index) {
        //Positive lits are colored with color0
        lit2color[current_node_index] = 0;
        ++current_node_index;
        //Negative lits are colored with color0
        lit2color[current_node_index] = 1;
    }
    colorcount.push_back(conf->nVars);
    colorcount.push_back(conf->nVars);

    //initialise the 4 extra nodes
    for (auto i : {2, 3, 4, 5}) {
        lit2color[current_node_index] = i;
        current_node_index++;
        colorcount.push_back(1);
    }

    //Initialise colorcount!!!
    colorcount.reserve(maxcolor);
    for (auto col = 6; col < maxcolor; col++) {
        colorcount.push_back(0);
    }

    uint32_t nbedges = 0;
    // Initialize edge lists
    // First construct for each node the list of neighbors
    vector<vector<uint32_t> > neighbours(n);
    // Literals have their negations as neighbors
    for (uint32_t l = 1; l <= conf->nVars; ++l) {
        uint32_t posID = encode(l);
        uint32_t negID = encode(-l);
        neighbours[posID].push_back(negID);
        neighbours[negID].push_back(posID);
        nbedges += 2;
    }

    for (auto r : rules) {
        auto bodyNode = current_node_index;
        current_node_index++;
        auto headNode = current_node_index;
        current_node_index++;

        lit2color[headNode] = 2;
        colorcount[2]++;

        if (r->ruleType == 1) {
            lit2color[bodyNode] = 3;
        } else if (r->ruleType == 3) {
            lit2color[bodyNode] = 4;
        } else if (r->ruleType == 2 || r->ruleType == 5) {
            auto color = boundToColor[r->bound];
            lit2color[bodyNode] = color;
        } else if (r->ruleType == 6) {
            lit2color[bodyNode] = 6;
        }

        colorcount[lit2color[bodyNode]]++;

        neighbours[bodyNode].push_back(headNode);
        neighbours[headNode].push_back(bodyNode);
        nbedges += 2;

        for (auto lit : r->headLits) {
            neighbours[lit].push_back(headNode);
            neighbours[headNode].push_back(lit);
            nbedges += 2;
        }
        if (r->ruleType != 5 && r->ruleType != 6) {
            for (auto lit : r->bodyLits) {
                neighbours[lit].push_back(bodyNode);
                neighbours[bodyNode].push_back(lit);
                nbedges += 2;
            }
        } else {
            for (u_int index = 0; index < r->bodyLits.size(); index++) {
                auto lit = r->bodyLits[index];
                auto weight = r->weights[index];
                auto extranode = current_node_index;
                current_node_index++;
                lit2color[extranode] = boundToColor[weight];
                neighbours[extranode].push_back(bodyNode);
                neighbours[bodyNode].push_back(extranode);
                neighbours[extranode].push_back(lit);
                neighbours[lit].push_back(extranode);
                nbedges += 4;
            }
        }
    }

    initializeGraph(n, nbedges, lit2color, neighbours);

    // look for unused lits, make their color unique so that no symmetries on them are found
    // useful for subgroups
    std::unordered_set<uint32_t> usedLits;
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
    for (uint32_t i = 0; i < 2 * conf->nVars; ++i) {
        if (usedLits.count(i) == 0) {
            setUniqueColor(i);
        }
    }

    // fix conf->fixedLits
    setUniqueColor(conf->fixedLits);
}

Graph::~Graph()
{
    freeGraph();
}

void Graph::print()
{
    for (size_t i = 0; i < getNbNodes(); ++i) {
        fprintf(stderr, "node %i with color %i has neighbours\n", (int)i,
                getColorOf(i));
        for (size_t j = 0; j < nbNeighbours(i); ++j) {
            fprintf(stderr, "%i ", getNeighbour(i, j));
        }
        fprintf(stderr, "\n");
    }
}

uint32_t Graph::getNbNodes()
{
    return getNbNodesFromGraph();
}

uint32_t Graph::getNbEdges()
{
    return getNbEdgesFromGraph();
}

void Graph::setUniqueColor(uint32_t lit)
{
    uint32_t mycolor = getColorOf(lit);
    uint32_t mycount = colorcount[getColorOf(lit)];
    assert(mycount > 0);
    if (mycount == 1) {
        return; // color was already unique
    }

    colorcount[mycolor]--;
    uint32_t new_color = colorcount.size();
    bliss_g->change_color(lit, new_color);
    color[lit] = new_color;
    colorcount.push_back(1);
}

void Graph::setUniqueColor(const vector<uint32_t>& lits)
{
    for (auto lit : lits) {
        setUniqueColor(lit);
    }
}

void Graph::getSymmetryGenerators(vector<shared_ptr<Permutation> >& out_perms)
{
    out_perms.clear();
    getSymmetryGeneratorsInternal(out_perms);
}
