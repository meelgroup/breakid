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

#ifndef BREAKID_H
#define BREAKID_H

#include <string>
#include <vector>
#include <unordered_map>
#include "breakid/solvertypesmini.hpp"

namespace BID {

struct PrivateData;

struct BreakID {
    BreakID();
    ~BreakID();

    std::string get_sha1_version() const;

    //configuration functions
    void set_useMatrixDetection(bool val);
    void set_useBinaryClauses(bool val);
    void set_useShatterTranslation(bool val);
    void set_useFullTranslation(bool val);
    void set_symBreakingFormLength(int val);
    void set_verbosity(uint32_t val);
    void set_steps_lim(int64_t val);

    //Dynamic CNF
    void start_dynamic_cnf(uint32_t nVars);
    void add_clause(BID::BLit* start, size_t num);
    void end_dynamic_cnf();

    //Main functions
    void detect_subgroups();
    void break_symm();

    //Print info
    void print_graph() const;
    void print_subgroups(std::ostream& out);
    void print_symm_break_stats();
    void print_perms_and_matrices(std::ostream& out);
    void print_generators(std::ostream& out);

    //Get info
    uint32_t get_num_generators();
    uint32_t get_num_break_cls();
    uint32_t get_num_aux_vars();
    int64_t get_steps_remain() const;
    uint64_t get_num_subgroups() const;
    std::vector<std::vector<BID::BLit>> get_brk_cls();
    void get_perms(std::vector<std::unordered_map<BLit, BLit> >* out);

private:
    PrivateData* dat = NULL;
};

}

#endif
