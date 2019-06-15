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

#ifndef ALGEBRAIC_H
#define ALGEBRAIC_H

#include <unordered_map>

#include "config.hpp"
#include "Breaking.hpp"

class Specification;
class Breaker;

class Permutation : public std::enable_shared_from_this<Permutation>
{
public:
    Permutation(Config* conf);
    Permutation(vector<std::pair<uint32_t, uint32_t> >& tuples, Config* conf);

    ///Permutation constructed from swapping two rows.
    Permutation(vector<uint32_t>& row1, vector<uint32_t>& row2, Config* conf);

    ~Permutation(){};

    uint32_t getImage(uint32_t from) const;
    // return value is true iff the image is different from the original
    bool getImage(vector<uint32_t>& orig, vector<uint32_t>& img) const;
    void getCycle(uint32_t lit, vector<uint32_t>& orb) const;
    bool isInvolution();
    bool permutes(uint32_t lit);
    uint32_t supportSize() const;
    bool isIdentity();

    void print(std::ostream& out) const;

    bool formsMatrixWith(shared_ptr<Permutation> other);
    std::pair<shared_ptr<Permutation>, shared_ptr<Permutation> > getLargest(
        shared_ptr<Permutation> other);
    void getSharedLiterals(shared_ptr<Permutation> other, vector<uint32_t>& shared);
    vector<uint32_t>& getCycleReprs() const;
    uint32_t getMaxCycleSize();
    uint32_t getNbCycles();

    bool equals(shared_ptr<Permutation> other);
    vector<uint32_t> domain;
    vector<uint32_t> posDomain;
    vector<uint32_t> image;

    void addFromTo(uint32_t from, uint32_t to);
    void addCycle(vector<uint32_t>& cyc);
    void addPrimeSplitToVector(vector<shared_ptr<Permutation> >& newPerms);
    const std::unordered_map<uint32_t, uint32_t>& getPerm();

private:
    std::unordered_map<uint32_t, uint32_t> perm;

    /// smallest lit in each cycle
    /// mutable because we only compute it once
    //  inside getCycleReprs(), the first time it's called
    mutable vector<uint32_t> cycleReprs;
    mutable uint32_t maxCycleSize;

    size_t hash;
    Config* conf;
};

class Matrix
{
   private:
    vector<vector<uint32_t>*>
        rows; // TODO: refactor this as 1 continuous vector
    std::unordered_map<uint32_t, uint32_t> rowco;
    std::unordered_map<uint32_t, uint32_t> colco;
    Config* conf;

   public:
    Matrix(Config* conf);
    ~Matrix();
    void print(std::ostream& out) const;

    void add(vector<uint32_t>* row);
    uint32_t nbColumns() const;
    uint32_t nbRows() const;
    void tryToAddNewRow(shared_ptr<Permutation> p, uint32_t rowIndex,
                        Specification* theory);
    vector<uint32_t>* getRow(uint32_t rowindex);
    bool permutes(uint32_t x);
    uint32_t getLit(uint32_t row, uint32_t column);

    uint32_t getRowNb(uint32_t x);
    uint32_t getColumnNb(uint32_t x);

    shared_ptr<Permutation> testMembership(const shared_ptr<Permutation> p);
    shared_ptr<Permutation> getProductWithRowsWap(const shared_ptr<Permutation> p, uint32_t r1,
                                            uint32_t r2); // return p*swap(r1,r2)
};

class Group
{
public:
    Specification* theory;
    void add_perms(vector<std::unordered_map<uint32_t, uint32_t>>& out);

    Group(Config* conf);

    void add(shared_ptr<Permutation> p);
    void checkColumnInterchangeability(shared_ptr<Matrix> m);
    void print(std::ostream& out) const;
    shared_ptr<Matrix> getInitialMatrix();
    void addMatrices();

    /// cnf-parameter, otherwise we have to store a pointer to the cnf here :(
    void addMatrix(shared_ptr<Matrix> m);
    uint32_t getNbMatrices();
    uint32_t getNbRowSwaps();

    shared_ptr<Matrix> getMatrix(uint32_t idx);

    void getDisjointGenerators(vector<Group*>& subgroups);
    uint32_t getSize();

    bool permutes(uint32_t lit);
    uint32_t getSupportSize();

    ///returns a vector containing a lit for literals relevant to construct sym breaking clauses
    void getOrderAndAddBinaryClausesTo(
        Breaker& brkr,
        vector<uint32_t>& out_order
    );

    void addBinaryClausesTo(Breaker& brkr, vector<uint32_t>& out_order,
                            const std::unordered_set<uint32_t>& excludedLits);
    void addBreakingClausesTo(Breaker& brkr);

    void maximallyExtend(shared_ptr<Matrix> matrix, uint32_t indexOfFirstNewRow);

private:
    vector<shared_ptr<Permutation> > permutations;
    vector<shared_ptr<Matrix> > matrices;
    std::unordered_set<uint32_t> support;

    ///remove permutations implied by the matrix
    void cleanPermutations(shared_ptr<Matrix> matrix);
    Config* conf;
};

inline const std::unordered_map<uint32_t, uint32_t>& Permutation::getPerm()
{
    return perm;
}

#endif
