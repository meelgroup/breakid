#pragma once

#include "global.hpp"

class Graph;
class Permutation;
class Group;
class Matrix;
class Breaker;

class Specification{
  friend class Breaker;
protected:
  sptr<Graph> graph;
  sptr<Group> group;
public:
  Specification();
  virtual ~Specification();

  virtual void print(std::ostream& out)=0;
  virtual uint getSize()=0;

  sptr<Graph> getGraph();
  sptr<Group> getGroup();

  virtual void cleanUp();

  virtual void setSubTheory(sptr<Group> subgroup)=0;

  virtual bool isSymmetry(Permutation& prm)=0;
};

class CNF: public Specification {
  friend class Breaker;

private:
  std::unordered_set<sptr<Clause>, UVecHash, UvecEqual> clauses; // must be an unordered_set, since we need to be able to test whether a clause exists to detect symmetries

  void readCNF(std::string& filename);

public:
  CNF(std::string& filename);
  CNF(std::vector<sptr<Clause> >& clss, sptr<Group> grp);
  ~CNF();

  void print(std::ostream& out);
  uint getSize();

  void setSubTheory(sptr<Group> subgroup);

  bool isSymmetry(Permutation& prm);
};


class LogicProgram: public Specification  {
  friend class Breaker;

private:
  std::unordered_set<sptr<Rule>, UVecHash, UvecEqual> rules; // must be an unordered_set, since we need to be able to test whether a rule exists to detect symmetries

public:
  LogicProgram(std::vector<sptr<Rule> >& rls, sptr<Group> grp);
  ~LogicProgram();

  void print(std::ostream& out);

  uint getSize();

  void setSubTheory(sptr<Group> subgroup);

  bool isSymmetry(Permutation& prm);
};


