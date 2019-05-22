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

#pragma once

#include "global.hpp"

class Specification;
class Breaker;

class Permutation : public std::enable_shared_from_this<Permutation>
{
   private:
    std::unordered_map<uint32_t, uint32_t> perm;
    std::vector<uint32_t> cycleReprs; // smallest lit in each cycle
    uint32_t maxCycleSize;
    size_t hash;

   public:
    std::vector<uint32_t> domain;
    std::vector<uint32_t> posDomain;
    std::vector<uint32_t> image;

    void addFromTo(uint32_t from, uint32_t to);
    void addCycle(std::vector<uint32_t>& cyc);
    void addPrimeSplitToVector(std::vector<sptr<Permutation> >& newPerms);

    Permutation();
    Permutation(std::vector<std::pair<uint32_t, uint32_t> >& tuples);
    // Permutation constructed from swapping two rows.
    Permutation(std::vector<uint32_t>& row1, std::vector<uint32_t>& row2);

    ~Permutation(){};

    uint32_t getImage(uint32_t from);
    // return value is true iff the image is different from the original
    bool getImage(std::vector<uint32_t>& orig, std::vector<uint32_t>& img);
    void getCycle(uint32_t lit, std::vector<uint32_t>& orb);
    bool isInvolution();
    bool permutes(uint32_t lit);
    uint32_t supportSize();
    bool isIdentity();

    void print(std::ostream& out);

    bool formsMatrixWith(sptr<Permutation> other);
    std::pair<sptr<Permutation>, sptr<Permutation> > getLargest(
        sptr<Permutation> other);
    void getSharedLiterals(sptr<Permutation> other, std::vector<uint32_t>& shared);
    std::vector<uint32_t>& getCycleReprs();
    uint32_t getMaxCycleSize();
    uint32_t getNbCycles();

    bool equals(sptr<Permutation> other);
};

class Matrix
{
   private:
    std::vector<std::vector<uint32_t>*>
        rows; // TODO: refactor this as 1 continuous vector
    std::unordered_map<uint32_t, uint32_t> rowco;
    std::unordered_map<uint32_t, uint32_t> colco;

   public:
    Matrix();
    ~Matrix();
    void print(std::ostream& out);

    void add(std::vector<uint32_t>* row);
    uint32_t nbColumns();
    uint32_t nbRows();
    void tryToAddNewRow(sptr<Permutation> p, uint32_t rowIndex,
                        Specification* theory);
    std::vector<uint32_t>* getRow(uint32_t rowindex);
    bool permutes(uint32_t x);
    uint32_t getLit(uint32_t row, uint32_t column);

    uint32_t getRowNb(uint32_t x);
    uint32_t getColumnNb(uint32_t x);

    sptr<Permutation> testMembership(const sptr<Permutation> p);
    sptr<Permutation> getProductWithRowsWap(const sptr<Permutation> p, uint32_t r1,
                                            uint32_t r2); // return p*swap(r1,r2)
};

class Group
{
   private:
    std::vector<sptr<Permutation> > permutations;
    std::vector<sptr<Matrix> > matrices;
    std::unordered_set<uint32_t> support;

    void cleanPermutations(
        sptr<Matrix> matrix); // remove permutations implied by the matrix

   public:
    // NOTE: if a group has a shared pointer to a theory, and a theory a shared pointer to a group, none of the memory pointed to by these pointers will ever be freed :(
    Specification* theory; // non-owning pointer

    void add(sptr<Permutation> p);
    void checkColumnInterchangeability(sptr<Matrix> m);

    void print(std::ostream& out);

    sptr<Matrix> getInitialMatrix();

    void addMatrices();
    void addMatrix(
        sptr<Matrix>
            m); // cnf-parameter, otherwise we have to store a pointer to the cnf here :(
    uint32_t getNbMatrices();
    uint32_t getNbRowSwaps();

    sptr<Matrix> getMatrix(uint32_t idx);

    void getDisjointGenerators(std::vector<sptr<Group> >& subgroups);
    uint32_t getSize();

    bool permutes(uint32_t lit);
    uint32_t getSupportSize();

    void getOrderAndAddBinaryClausesTo(
        Breaker& brkr,
        std::vector<uint32_t>&
            out_order); // returns a vector containing a lit for literals relevant to construct sym breaking clauses
    void addBinaryClausesTo(Breaker& brkr, std::vector<uint32_t>& out_order,
                            const std::unordered_set<uint32_t>& excludedLits);
    void addBreakingClausesTo(Breaker& brkr);

    void maximallyExtend(sptr<Matrix> matrix, uint32_t indexOfFirstNewRow);
};
