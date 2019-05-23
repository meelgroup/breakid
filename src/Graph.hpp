/* Graph is a class used as a wrapper for saucy methods
 */

#ifndef BREAKID_GRAPH_H
#define BREAKID_GRAPH_H

#include "Algebraic.hpp"
#include "global.hpp"

class Matrix;
class Group;

namespace bliss {
class Graph;
}

class Graph : public std::enable_shared_from_this<Graph>
{
   private:
   public:
    bliss::Graph* bliss_g;

    vector<uint32_t>
        colorcount; // keeps track of the number of times a color is used, so that no color is never used (seems to give Saucy trouble)
    // @INVAR: for all x: colorcount[x]>0

    Graph(std::unordered_set<sptr<Clause>, UVecHash, UvecEqual>& clauses);
    Graph(std::unordered_set<sptr<Rule>, UVecHash, UvecEqual>& rules);
    ~Graph();

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
        vector<sptr<Permutation> >& out_perms);

   public:
    uint32_t getNbNodes();
    uint32_t getNbEdges();
    void print();
    void setUniqueColor(uint32_t lit);
    void setUniqueColor(const vector<uint32_t>& lits);
    void getSymmetryGenerators(vector<sptr<Permutation> >& out_perms);
};

#endif
