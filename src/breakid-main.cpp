/******************************************
Copyright (c) 2021 Mate Soos

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
#include "config.hpp"
#include "argparse.hpp"

using std::cout;
using std::cerr;
using std::string;
using std::endl;
using BID::BLit;
using std::vector;
using std::istringstream;
Config conf;
argparse::ArgumentParser program = argparse::ArgumentParser("breakid");
uint32_t nVars;


void add_options()
{
    program.add_argument("-h", "--help")
        .help("Print help")
        .default_value(false);
    program.add_argument("-v", "--version")
        .help("Print version info")
        .flag();
    program.add_argument("--verb")
        .action([&](const auto& a) {conf.verbosity = std::atoi(a.c_str());})
        .default_value(conf.verbosity)
        .help("[0-10] Verbosity");

    program.add_argument("--row")
        .action([&](const auto& a) {conf.useMatrixDetection = std::atoi(a.c_str());})
        .default_value(conf.useMatrixDetection)
        .help("Enable/disable detection and breaking of row interchangeability");

    program.add_argument("--bin")
        .action([&](const auto& a) {conf.useBinaryClauses = std::atoi(a.c_str());})
        .default_value(conf.useBinaryClauses)
        .help("Use/don't use construction of additional binary symmetry breaking clauses"
                "based on stabilizer subgroups");

    program.add_argument("-s")
        .action([&](const auto& a) {conf.symBreakingFormLength = std::atoi(a.c_str());})
        .default_value(conf.symBreakingFormLength)
        .help("Limit the size of the constructed symmetry breaking"
            " formulas, measured as the number of auxiliary variables");

    program.add_argument("-t")
        .action([&](const auto& a) {
                conf.steps_lim = std::atoll(a.c_str());
                conf.steps_lim *= 1000LL;
                })
        .default_value(conf.steps_lim)
        .help("Upper limit on computing steps spent in kilo-steps, approximate measure for time");

    program.add_argument("--small")
        .action([&](const auto& a) {conf.useShatterTranslation = std::atoi(a.c_str());})
        .default_value(conf.useShatterTranslation)
        .help("Enable/disable compact symmetry breaking encoding, use Shatter's encoding instead");

    program.add_argument("--relaxed")
        .action([&](const auto& a) {conf.useFullTranslation = std::atoi(a.c_str());})
        .default_value(conf.useFullTranslation)
        .help("Enable/disable relaxing constraints on auxiliary encoding"
                "variables, use longer encoding instead");

    program.add_argument("files").remaining().help("input and optionally file");
}

inline uint32_t encode(int lit) {
    return (lit > 0 ? 2 * (lit - 1) : 2 * (-lit - 1) + 1);
}

vector<vector<BLit>> readCNF(const char* filename)
{
    std::ifstream file(filename);
    if (!file) {
        cerr << "ERROR: No CNF file found." << endl;
        exit(-1);
    }

    vector<vector<BLit>> clauses;
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
                        cerr << "ERROR: Theory can not contain empty clause." << endl;
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
    add_options();
    try {
        program.parse_args(argc, argv);
        if (program.is_used("--help")) {
            cout << "Symmetry breaking tool. " << endl;
            cout << "breakid [options] inputfile outputfile" << endl << endl;
            cout << program << endl;
            std::exit(0);
        }
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << endl;
        std::cerr << program;
        exit(-1);
    }
    if (program["version"] == true) {
        cout << "c BreakID SHA1: " << BID::BreakID::get_version_sha1() << endl;
        std::exit(0);
    }

    BID::BreakID breakid;
    breakid.set_useMatrixDetection(conf.useMatrixDetection);
    breakid.set_useBinaryClauses(conf.useBinaryClauses);
    breakid.set_useShatterTranslation(conf.useShatterTranslation);
    breakid.set_useFullTranslation(conf.useFullTranslation);
    breakid.set_symBreakingFormLength(conf.symBreakingFormLength);
    breakid.set_verbosity(conf.verbosity);
    breakid.set_verbosity(conf.verbosity);
    breakid.set_steps_lim(conf.steps_lim);

    std::string in_fname;
    std::string out_fname;
    try {
        auto files = program.get<std::vector<std::string>>("files");
        if (files.size() > 2) {
            cerr << "ERROR: you can only have at most two files as positional options:"
                "the input file and the output file" << endl;
            exit(-1);
        }
        if (files.empty()) {
            cout << "ERROR: You must give at least an input CNF file" << endl;
            exit(-1);
        }
        in_fname = files[0];
        if (files.size() > 1) out_fname = files[1];
    } catch (std::logic_error& e) {
        cout << "ERROR parsing option: "<< e.what() << endl;
        cout << "ERROR: Did you forget to add an input CNF file?" << endl;
        exit(-1);
    }

    if (conf.verbosity) cout << "c BreakID version " << breakid.get_sha1_version() << endl;

    vector<vector<BLit>> cls = readCNF(in_fname.c_str());
    breakid.start_dynamic_cnf(nVars);
    for(auto cl: cls) breakid.add_clause(cl.data(), cl.size());

    ////////////////
    //Find symmetry group
    ////////////////
    double myTime = cpuTime();
    breakid.end_dynamic_cnf();
    int64_t remain = breakid.get_steps_remain();
    bool timeout = remain <= 0;
    double remain_ratio = (double)remain/(double)conf.steps_lim;
    if (conf.verbosity) {
        cout << "c Finished symmetry breaking. T: "
        << std::setprecision(2)
        << std::fixed
        << (cpuTime()-myTime) << " s "
        << " T-out: " << (timeout? "Y" : "N")
        << " T-rem: " << remain_ratio
        << endl;
    }
    if (conf.verbosity >= 1) cout << "c Num generators: " << breakid.get_num_generators() << endl;
    if (conf.verbosity >= 2) breakid.print_generators(cout);

    ////////////////
    // Deal with subgroups
    ////////////////
    if (conf.verbosity) cout << "c Detecting subgroups..." << endl;
    myTime = cpuTime();
    breakid.detect_subgroups();
    if (conf.verbosity) {
        cout << "c Finding subgroups breaking. T: "
        << std::setprecision(2)
        << std::fixed
        << (cpuTime()-myTime) << " s "
        << endl;
    }
    if (conf.verbosity) cout << "c num subgroups: " << breakid.get_num_subgroups() << endl;
    if (conf.verbosity >= 2) breakid.print_subgroups(cout);

    ////////////////
    // Break symmetries
    ////////////////
    breakid.break_symm();
    if (conf.verbosity) breakid.print_symm_break_stats();

    std::ostream* out = &std::cout;
    if (!out_fname.empty()) {
        std::ofstream* f = new std::ofstream;
        f->open(out_fname.c_str());
        out = f;
    }
    *out << "p cnf " << (nVars + breakid.get_num_aux_vars())
        << " " << (cls.size() + breakid.get_brk_cls().size()) << endl;
    for(auto cl: cls) {
        for(auto lit: cl) *out << lit << " ";
        *out << "0" << endl;
    }
    auto brk = breakid.get_brk_cls();
    for (auto cl: brk) {
        for(auto lit: cl) *out << lit << " ";
        *out << "0" << endl;
    }
    if (!out_fname.empty()) delete out;
    return 0;
}
