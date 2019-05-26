/* Graph is a class used as a wrapper for saucy methods
 */

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

    Graph(std::unordered_set<shared_ptr<Clause>, UVecHash, UvecEqual>& clauses, Config* conf);
    Graph(std::unordered_set<shared_ptr<Rule>, UVecHash, UvecEqual>& rules, Config* conf);
    ~Graph();

    //Dynamic graph
    Graph(uint32_t nClauses, Config* conf); ///<for online CNF
    void add_clause(BID::BLit* start, uint32_t size);
    void end_dynamic_cnf();

    uint32_t getNbNodes();
    uint32_t getNbEdges();
    void print();
    void setUniqueColor(uint32_t lit);
    void setUniqueColor(const vector<uint32_t>& lits);
    void getSymmetryGenerators(vector<shared_ptr<Permutation> >& out_perms);

    //TODO should be private
    vector<shared_ptr<Permutation> > perms;
    Config* conf = NULL;

private:
    //Interaction with saucy:
    void initializeGraph(uint32_t nbNodes, uint32_t nbEdges,
                         std::map<uint32_t, uint32_t>& lit2color,
                         vector<vector<uint32_t> >& neighbours);
    void freeGraph();
    uint32_t getNbNodesFromGraph();
    uint32_t getNbEdgesFromGraph();
    uint32_t getColorOf(uint32_t node);
    uint32_t nbNeighbours(uint32_t node);
    uint32_t getNeighbour(uint32_t node, uint32_t nbthNeighbour);
    void setNodeToNewColor(uint32_t node);
    void getSymmetryGeneratorsInternal(
        vector<shared_ptr<Permutation> >& out_perms);

    vector<uint32_t> color;

    //dynamic CNF generation
    uint32_t cur_cl_num = 0;
    vector<char> used_lits;
};

#endif
