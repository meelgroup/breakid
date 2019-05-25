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

#include "config.hpp"
#include "Breaking.hpp"
#include "breakid/solvertypesmini.hpp"

class Graph;
class Permutation;
class Group;
class Matrix;
class Breaker;

class Specification
{
    friend class Breaker;

   protected:
    shared_ptr<Graph> graph;
    shared_ptr<Group> group;

   public:
    Specification();
    virtual ~Specification();

    virtual void print(std::ostream& out) = 0;
    virtual uint32_t getSize() = 0;

    shared_ptr<Graph> getGraph();
    shared_ptr<Group> getGroup();

    virtual void add_clause(BID::Lit* lits, uint32_t size)
    {}
    virtual void add_bin_clause(BID::Lit lit1, BID::Lit lit2)
    {}
    virtual void cleanUp();
    virtual void setSubTheory(shared_ptr<Group> subgroup) = 0;
    virtual bool isSymmetry(Permutation& prm) = 0;
};

class CNF : public Specification
{
public:
    CNF(string& filename, Config* conf);
    CNF(vector<shared_ptr<Clause> >& clss, shared_ptr<Group> grp);
    ~CNF();

    void print(std::ostream& out);
    uint32_t getSize();
    void setSubTheory(shared_ptr<Group> subgroup);
    bool isSymmetry(Permutation& prm);
    friend class Breaker;

private:
    ///must be an unordered_set, since we need to be able to test
    ///whether a clause exists to detect symmetries
    std::unordered_set<shared_ptr<Clause>, UVecHash, UvecEqual> clauses;

    void readCNF(string& filename);
    Config* conf;
};

class OnlCNF : public Specification
{
public:
    OnlCNF(uint32_t nVars, uint32_t num_cls, Config* conf);
    ~OnlCNF();

    void post_graph_run();
    void add_clause(BID::Lit* lits, uint32_t size);
    void add_bin_clause(BID::Lit lit1, BID::Lit lit2);

    void print(std::ostream& out);
    uint32_t getSize();
    void setSubTheory(shared_ptr<Group> subgroup);
    bool isSymmetry(Permutation& prm);
    friend class Breaker;

private:
    Config* conf;
    uint32_t num_cls;
};

class LogicProgram : public Specification
{
public:
    LogicProgram(vector<shared_ptr<Rule> >& rls, shared_ptr<Group> grp, Config* conf);
    ~LogicProgram();

    void print(std::ostream& out);
    uint32_t getSize();
    void setSubTheory(shared_ptr<Group> subgroup);
    bool isSymmetry(Permutation& prm);
    friend class Breaker;

private:

    ///Must be an unordered_set, since we need to be able to test
    ///whether a rule exists to detect symmetries
    std::unordered_set<shared_ptr<Rule>, UVecHash, UvecEqual> rules;
    Config* conf;
};

#endif
