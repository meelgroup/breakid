/******************************************
Copyright (c) 2019 Jo Devriendt - KU Leuven
Copyright (c) 2019 Bart Bogaerts

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

Graph::Graph(Config* _conf) :
    conf(_conf)
{
    uint32_t n = 2 * conf->nVars;
    bliss_g = new bliss::Graph(n);
    used_lits.resize(conf->nVars*2, 0);
    neighbours.clear();
    lit2color.clear();
    nbedges = 0;
    vertex_to_color.clear();

    // Initialize colors
    for (uint32_t i = 0; i < 2 * conf->nVars; ++i) {
        bliss_g->change_color(i, 0);
        vertex_to_color.push_back(0);
    }
    colorcount.push_back(2 * conf->nVars);
    colorcount.push_back(100); //we expect there to be many clauses;

    // Initialize edge lists
    // First construct for each node the list of neighbors
    // Literals have their negations as neighbors
    for (uint32_t l = 0; l < conf->nVars; l++) {
        uint32_t posID = BLit(l, false).toInt();
        uint32_t negID = BLit(l, true).toInt();
        bliss_g->add_edge(posID, negID);
        nbedges += 1;
    }
    assert(vertex_to_color.size() == n);
    assert(bliss_g->get_nof_vertices() == n);
}

void Graph::add_clause(BID::BLit* lits, uint32_t size)
{
    assert(size > 0 && "Must have clauses of size at least 1");

    if (size == 1) {
        BID::BLit lit = lits[0];
        setUniqueColor(lit.toInt());
    }

    // Clauses have as neighbors the literals occurring in them
    uint32_t v = bliss_g->add_vertex(1);
    for(size_t i = 0; i < size; i++) {
        BID::BLit l = lits[i];
        bliss_g->add_edge(v, l.toInt());
        used_lits[l.toInt()] = 1;
        nbedges+=1;
    }
    assert(vertex_to_color.size() == v);
    vertex_to_color.push_back(1);
}

void Graph::end_dynamic_cnf()
{
    colorcount[1] = bliss_g->get_nof_vertices() - conf->nVars*2;

    // look for unused lits, make their color unique so that no symmetries on them are found
    // useful for subgroups
    for (uint32_t i = 0; i < 2 * conf->nVars; ++i) {
        if (used_lits[i] == 0) {
            setUniqueColor(i);
        }
    }
}

void Graph::initializeGraph(uint32_t nbNodes)
{
    delete bliss_g;
    bliss_g = new bliss::Graph(nbNodes);
    vertex_to_color.resize(nbNodes);

    for (size_t n = 0; n < nbNodes; n++) {
        bliss_g->change_color(n, lit2color[n]);
        vertex_to_color[n] = lit2color[n];
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

uint32_t Graph::getColorOf(uint32_t node)
{
    assert(node < vertex_to_color.size());
    return vertex_to_color[node];
}

uint32_t Graph::nbNeighbours(uint32_t /*node*/)
{
    return 0; //Not supported, only used for printing
}
uint32_t Graph::getNeighbour(uint32_t /*node*/, uint32_t /*nbthNeighbour*/)
{
    return 0; //Not supported, only used for printing
}

///This method is given to BLISS as a polymorphic consumer
///of the detected generator permutations
static void addBlissPermutation(
    void* param, const unsigned int n,
    const unsigned int* aut)
{
    Graph* g = (Graph*)param;

    shared_ptr<Permutation> permu = std::make_shared<Permutation>(g->conf);
    for (unsigned i = 0; i < n; ++i) {
        if (i != aut[i]) {
            permu->addFromTo(BLit::toBLit(i), BLit::toBLit(aut[i]));
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

    lit2color.clear();

    // Initialize colors:
    for (uint32_t i = 0; i < 2 * conf->nVars; ++i) {
        lit2color[i] = 0;
    }
    colorcount.push_back(2 * conf->nVars);

    for (int i = 2 * conf->nVars; i < n; ++i) {
        lit2color[i] = 1;
    }
    colorcount.push_back(clauses.size());

    nbedges = 0;

    // Initialize edge lists
    // First construct for each node the list of neighbors
    neighbours.clear();
    neighbours.resize(n);
    // Literals have their negations as neighbors
    for (uint32_t l = 0; l < conf->nVars; l++) {
        uint32_t posID = BLit(l, false).toInt();
        uint32_t negID = BLit(l, true).toInt();
        neighbours[posID].push_back(negID);
        neighbours[negID].push_back(posID);
        nbedges += 2;
    }
    // Clauses have as neighbors the literals occurring in them
    uint32_t c = 2 * conf->nVars;
    for (auto cl : clauses) {
        for (auto lit : cl->lits) {
            neighbours[lit.toInt()].push_back(c);
            neighbours[c].push_back(lit.toInt());
            nbedges += 2;
        }
        ++c;
    }

    //Now, initialize the internal graph
    initializeGraph(n);

    // look for unused lits, make their color unique so that no symmetries on them are found
    // useful for subgroups
    used_lits.clear();
    used_lits.resize(2*conf->nVars, 0);
    for (auto cl : clauses) {
        for (auto lit : cl->lits) {
            used_lits[lit.toInt()] = 1;
            used_lits[(~lit).toInt()] = 1;
        }
    }
    for (uint32_t i = 0; i < 2 * conf->nVars; ++i) {
        if (used_lits[i] == 0) {
            setUniqueColor(i);
        }
    }
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
    vertex_to_color[lit] = new_color;
    colorcount.push_back(1);
}

void Graph::setUniqueColor(const vector<BLit>& lits)
{
    for (auto lit : lits) {
        setUniqueColor(lit.toInt());
    }
}

void Graph::getSymmetryGenerators(vector<shared_ptr<Permutation> >& out_perms)
{
    out_perms.clear();
    getSymmetryGeneratorsInternal(out_perms);
}
