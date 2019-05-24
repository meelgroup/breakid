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

#ifndef THEORY_H
#define THEORY_H

#include "global.hpp"

class Graph;
class Permutation;
class Group;
class Matrix;
class Breaker;

class Specification
{
    friend class Breaker;

   protected:
    sptr<Graph> graph;
    sptr<Group> group;

   public:
    Specification();
    virtual ~Specification();

    virtual void print(std::ostream& out) = 0;
    virtual uint32_t getSize() = 0;

    sptr<Graph> getGraph();
    sptr<Group> getGroup();

    virtual void cleanUp();
    virtual void setSubTheory(sptr<Group> subgroup) = 0;
    virtual bool isSymmetry(Permutation& prm) = 0;
};

class CNF : public Specification
{
public:
    CNF(string& filename, Config* conf);
    CNF(vector<sptr<Clause> >& clss, sptr<Group> grp);
    ~CNF();

    void print(std::ostream& out);
    uint32_t getSize();
    void setSubTheory(sptr<Group> subgroup);
    bool isSymmetry(Permutation& prm);
    friend class Breaker;

private:
    ///must be an unordered_set, since we need to be able to test
    ///whether a clause exists to detect symmetries
    std::unordered_set<sptr<Clause>, UVecHash, UvecEqual> clauses;

    void readCNF(string& filename);
    Config* conf;
};

/*class OnlCNF : public Specification
{
public:
    OnlCNF(uint32_t nVars, Config* conf);
    ~OnlCNF();

    void post_graph_run();
    void addClause(void* lits, uint32_t size);
    void addBinClause(void* lit1, void* lit2);

    void print(std::ostream& out);
    uint32_t getSize();
    void setSubTheory(sptr<Group> subgroup);
    bool isSymmetry(Permutation& prm);
    friend class Breaker;

private:
    Config* conf;
};*/

class LogicProgram : public Specification
{
public:
    LogicProgram(vector<sptr<Rule> >& rls, sptr<Group> grp, Config* conf);
    ~LogicProgram();

    void print(std::ostream& out);
    uint32_t getSize();
    void setSubTheory(sptr<Group> subgroup);
    bool isSymmetry(Permutation& prm);
    friend class Breaker;

private:

    ///Must be an unordered_set, since we need to be able to test
    ///whether a rule exists to detect symmetries
    std::unordered_set<sptr<Rule>, UVecHash, UvecEqual> rules;
    Config* conf;
};

#endif
