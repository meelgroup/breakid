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
#include <iomanip>

#include "breakid.hpp"
#include "time_mem.h"
#include "breakid/solvertypesmini.hpp"

using std::cout;
using std::string;
using std::endl;
using BID::BLit;
using std::vector;
using std::istringstream;

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
    if (argc != 3)  {
        cout << "You must give the CNF as 1st and max steps as second parameter" << endl;
        exit(-1);
    }

    BID::BreakID breakid;
    int conf_verbosity = 1;
    breakid.set_verbosity(conf_verbosity);

    //Steps
    int64_t steps_lim = atoi(argv[2]);
    steps_lim *= 1LL*1000LL;
    cout << "Limit: " << steps_lim << endl;
    breakid.set_steps_lim(steps_lim);
    breakid.set_useMatrixDetection(true);

    vector<vector<BLit>> cls = readCNF(argv[1]);
    cout << "OK, read " << cls.size() << " clauses" << endl;

    breakid.start_dynamic_cnf(nVars);
    for(auto cl: cls) {
        breakid.add_clause(cl.data(), cl.size());
    }

    ////////////////
    //Find symmetry group
    ////////////////
    double myTime = cpuTime();
    breakid.end_dynamic_cnf();
    int64_t remain = breakid.get_steps_remain();
    bool timeout = remain <= 0;
    double remain_ratio = (double)remain/(double)steps_lim;
    if (conf_verbosity) {
        cout << "-> Finished symmetry breaking. T: "
        << std::setprecision(2)
        << std::fixed
        << (cpuTime()-myTime) << " s "
        << " T-out: " << (timeout? "Y" : "N")
        << " T-rem: " << remain_ratio
        << endl;
    }
    if (conf_verbosity >= 1) {
        cout << "-> Num generators: " << breakid.get_num_generators() << endl;
    }
    if (conf_verbosity >= 2) {
        breakid.print_generators(std::cout);
    }

    ////////////////
    // Deal with subgroups
    ////////////////
    if (conf_verbosity) {
        cout << "Detecting subgroups..." << endl;
    }
    myTime = cpuTime();
    breakid.detect_subgroups();
    if (conf_verbosity) {
        cout << "-> Finding subgroups breaking. T: "
        << std::setprecision(2)
        << std::fixed
        << (cpuTime()-myTime) << " s "
        << endl;
    }
    if (conf_verbosity) {
        cout << "-> num subgroups: " << breakid.get_num_subgroups() << endl;
    }
    if (conf_verbosity >= 2) {
        breakid.print_subgroups(cout);
    }

    ////////////////
    // Break symmetries
    ////////////////
    breakid.break_symm();
    if (false && conf_verbosity) {
        breakid.print_symm_break_stats();
    }

    cout << "Num breaking clasues: "<< breakid.get_num_break_cls() << endl;
    cout << "Num aux vars: "<< breakid.get_num_aux_vars() << endl;
    if (conf_verbosity > 3) {
        auto brk = breakid.get_brk_cls();
        for (auto cl: brk) {
            cout << "Breaking clause: ";
            for(auto lit: cl) {
                cout << lit << " ";
            }
            cout << endl;
        }
    }

    return 0;
}
