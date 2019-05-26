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

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <limits>
#include <fstream>
#include <set>
#include <sstream>
#include "breakid/solvertypesmini.hpp"

using std::cout;
using std::string;
using std::endl;
using BID::BLit;
using std::vector;
using std::istringstream;

#include "breakid.hpp"

uint32_t nVars;

inline uint32_t encode(int lit)
{
    return (lit > 0 ? 2 * (lit - 1) : 2 * (-lit - 1) + 1);
}

vector<vector<BLit>> readCNF(const char* filename)
{
    vector<vector<BLit>> clauses;
    //cout << "*** Reading CNF: " << filename << endl;

    std::ifstream file(filename);
    if (!file) {
        std::cout << "ERROR: No CNF file found." << endl;
        exit(-1);
    }
    string line;
    vector<BLit> inclause;
    while (getline(file, line)) {
        if (line.size() == 0 || line.front() == 'c') {
            // do nothing, this is a comment line
        } else if (line.front() == 'p') {
            string info = line.substr(6);
            istringstream iss(info);
            uint32_t nbVars;
            iss >> nbVars;
            uint32_t nbClauses;
            iss >> nbClauses;
            nVars = nbVars;
            clauses.reserve(nbClauses);
        } else {
            //  parse the clauses, removing tautologies and double lits
            istringstream iss(line);
            int l;
            while (iss >> l) {
                if (l == 0) {
                    if (inclause.size() == 0) {
                        std::cout << "ERROR: Theory can not contain empty clause." << endl;
                        exit(-1);
                    }
                    clauses.push_back(inclause);
                    inclause.clear();
                } else {
                    if ((uint32_t)abs(l) > nVars) {
                        nVars = abs(l);
                    }
                    inclause.push_back(BLit(abs(l)-1, l < 0));
                }
            }
        }
    }

    return clauses;
}

int main(int argc, char *argv[])
{
    BID::BreakID breakid;

    int conf_verbosity = 3;
    breakid.set_verbosity(conf_verbosity);
    vector<vector<BLit>> cls = readCNF(argv[1]);
    cout << "OK, read " << cls.size() << " clauses" << endl;

    breakid.start_dynamic_cnf(nVars, cls.size());
    for(auto cl: cls) {
        breakid.add_clause(cl.data(), cl.size());
    }
    breakid.end_dynamic_cnf();


    /*breakid.start_dynamic_cnf(2, 2);
    breakid.add_bin_clause(BLit(0, false), BLit(1, false));
    breakid.add_bin_clause(BLit(0, true), BLit(1, true));
    breakid.end_dynamic_cnf();*/

    if (conf_verbosity > 3) {
        breakid.print_graph();
    }

    if (conf_verbosity) {
        cout << "Num generators: " << breakid.get_num_generators() << endl;
        breakid.print_generators();
    }

    if (conf_verbosity >= 2) {
        cout << "*** Detecting subgroups..." << endl;
    }
    breakid.detect_subgroups();

    if (conf_verbosity) {
        breakid.print_subgroups();
    }

    breakid.clean_theory();
    breakid.break_symm();

    if (false && conf_verbosity) {
        breakid.print_symm_break_stats();
        breakid.write_final_cnf(true);
    }

    cout << "Num breaking clasues: "<< breakid.get_num_break_cls() << endl;
    cout << "Num aux vars: "<< breakid.get_num_aux_vars() << endl;
    auto brk = breakid.get_brk_cls();
    for (auto cl: brk) {
        cout << "Breaking clause: ";
        for(auto lit: cl) {
            cout << lit << " ";
        }
        cout << endl;
    }

    if (true) {
        string symFile = "test.sym";
        if (conf_verbosity) {
            cout
            << "*** Printing generators to file " + symFile
            << endl;
        }

        breakid.print_generators(symFile);
    }

    return 0;
}
