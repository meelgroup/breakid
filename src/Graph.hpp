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

#ifndef BREAKID_GRAPH_H
#define BREAKID_GRAPH_H

#include <map>

#include "Algebraic.hpp"
#include "Breaking.hpp"
#include "breakid/solvertypesmini.hpp"

class Matrix;
class Group;

namespace bliss {
class Graph;
}

class Graph : public std::enable_shared_from_this<Graph>
{
public:
    bliss::Graph* bliss_g = NULL;

    ///keeps track of the number of times a color is used
    ///so that no color is never used (seems to give Saucy trouble)
    ///@INVAR: for all x: colorcount[x]>0
    vector<uint32_t> colorcount;
    ~Graph();

    //Dynamic graph
    Graph(Config* conf); ///<for online CNF
    void add_clause(BID::BLit* start, uint32_t size);
    void end_dynamic_cnf();

    uint32_t getNbNodes() const;
    void setUniqueColor(uint32_t lit);
    void setUniqueColor(const vector<BLit>& lits);
    void getSymmetryGenerators(
        vector<shared_ptr<Permutation> >& out_perms
        , int64_t steps_lim
        , int64_t* out_steps_lim
    );

    //TODO should be private
    vector<shared_ptr<Permutation> > perms;
    Config* conf = NULL;

private:
    //Interaction with saucy:
    void initializeGraph(uint32_t nbNodes);
    void freeGraph();
    uint32_t getNbNodesFromGraph() const;
    uint32_t getColorOf(uint32_t node) const;
    void setNodeToNewColor(uint32_t node);
    void getSymmetryGeneratorsInternal(
        vector<shared_ptr<Permutation> >& out_perms
        , int64_t steps_lim
        , int64_t* out_steps_lim
    );

    vector<uint32_t> vertex_to_color;

    std::map<uint32_t, uint32_t> lit2color;
    vector<vector<uint32_t> > neighbours;
    uint32_t nbedges = 0;

    //dynamic CNF generation
    vector<char> used_lits;
};

#endif
