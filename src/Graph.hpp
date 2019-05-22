/* Graph is a class used as a wrapper for saucy methods
 */

#pragma once

#define BLISS
#undef SAUCY
#undef NAUTY
#undef TRACES

#include "Algebraic.hpp"
#include "global.hpp"
#if defined(NAUTY) || defined(TRACES)
//TODO/ only for  struct sparsegraph;
extern "C" {
#include "nauty/nausparse.h"
}
#endif

// TODO: make a die function for mem-assignment issues?
/*static void die(string& s){
  std::clog << s << endl;
  exit(1);
}*/

class Matrix;
class Group;

#ifdef SAUCY
struct saucy_graph;
#endif

//#ifdef NAUTY
//struct sparsegraph;
//#endif

#ifdef BLISS
namespace bliss {
class Graph;
}
#endif

class Graph : public std::enable_shared_from_this<Graph>
{
   private:
   public:
#ifdef SAUCY
    saucy_graph* saucy_g;
#endif
#if defined(NAUTY) || defined(TRACES)
    sparsegraph* sparse_graph;
    //Colorings:
    int* lab;
    int* ptn;
    std::map<uint32_t, uint32_t>
        node2pos; //maps literal to their position in the color vector
#endif
#ifdef BLISS
    bliss::Graph* bliss_g;
#endif
    std::vector<uint32_t>
        colorcount; // keeps track of the number of times a color is used, so that no color is never used (seems to give Saucy trouble)
    // @INVAR: for all x: colorcount[x]>0

    Graph(std::unordered_set<sptr<Clause>, UVecHash, UvecEqual>& clauses);
    Graph(std::unordered_set<sptr<Rule>, UVecHash, UvecEqual>& rules);
    ~Graph();

   private:
    //Interaction with saucy:
    void initializeGraph(uint32_t nbNodes, uint32_t nbEdges,
                         std::map<uint32_t, uint32_t>& lit2color,
                         std::vector<std::vector<uint32_t> >& neighbours);
    void freeGraph();
    uint32_t getNbNodesFromGraph();
    uint32_t getNbEdgesFromGraph();
    uint32_t getColorOf(uint32_t node);
    uint32_t nbNeighbours(uint32_t node);
    uint32_t getNeighbour(uint32_t node, uint32_t nbthNeighbour);
    void setNodeToNewColor(uint32_t node);
    void getSymmetryGeneratorsInternal(
        std::vector<sptr<Permutation> >& out_perms);

   public:
    uint32_t getNbNodes();
    uint32_t getNbEdges();
    void print();
    void setUniqueColor(uint32_t lit);
    void setUniqueColor(const std::vector<uint32_t>& lits);
    void getSymmetryGenerators(std::vector<sptr<Permutation> >& out_perms);
};
