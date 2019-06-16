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
#include <iostream>
#include <limits>
#include <fstream>

using std::cout;
using std::string;
using std::endl;

#include "breakid.hpp"
#include "config.hpp"

bool printGeneratorFile = false;
bool onlyPrintBreakers = false;
Config conf;

namespace options {
    string intch_symm = "--row";
    string nobinary =   "--no-bin";
    string formlength = "-s";
    string verbosity = "-v";
    string steps_lim = "-t";
    string help = "-h";
    string nosmall = "--no-small";
    string norelaxed = "--no-relaxed";
    string onlybreakers = "--only-brk";
    string generatorfile = "--with-gen-fil";
}

void printUsage()
{
    BID::BreakID bid;
    cout << "BreakID version " << bid.get_sha1_version() << endl;
    cout << "Usage: ./BreakID <cnf-file> "
              << "[" << options::help << "] "
              << "[" << options::intch_symm << "] "
              << "[" << options::nobinary << "] "
              << "[" << options::nosmall << "] "
              << "[" << options::norelaxed << "] "
              << "[" << options::formlength << " <nat>] "
              << "[" << options::steps_lim << " <nat>] "
              << "[" << options::verbosity << " <nat>] "
              << "[" << options::onlybreakers << "] "
              << "[" << options::generatorfile << "] "
              << "\n";

    cout
    << "\nOptions:\n"
    << options::help << "\n"
    << "    Display this help message instead of running BreakID.\n\n"

    << options::intch_symm << "\n"
    << "    ENABLE detection and breaking of row interchangeability.\n\n"

    << options::nobinary << "\n"
    << "    Disable construction of additional binary symmetry breaking\n\n"
    << "    clauses based on stabilizer subgroups.\n"

    << options::nosmall << "\n"
    << "    Disable compact symmetry breaking encoding, use Shatter's\n\n"
    << "    encoding instead.\n"

    << options::norelaxed << "\n"
    << "    Disable relaxing constraints on auxiliary encoding\n"
    << "    variables, use longer encoding instead.\n\n"

    << options::formlength << " <default: " << conf.symBreakingFormLength << ">\n"
    << "    Limit the size of the constructed symmetry breaking\n"
    << "    formula's, measured as the number of auxiliary variables\n"
    << "    introduced. <-1> means no symmetry breaking.\n\n"

    << options::steps_lim << " <default: " << conf.steps_lim << ">\n"
    << "    Upper limit on computing steps spent, approximate measure for time.\n\n"

    << options::verbosity << " <default: " << conf.verbosity << ">\n"
    << "    Verbosity of the output. <0> means no output other than the\n"
    << "    CNF augmented with symmetry breaking clauses.\n\n"

    << options::onlybreakers << "\n"
    << "    Do not print original theory, only the symmetry breaking\n"
    << "    clauses.\n\n"

    << options::generatorfile << "\n"
    << "    Return the generator symmetries as a <path-to-cnf>.sym file.\n\n"
    ;
}

void parseOptions(int argc, char *argv[])
{
    if (argc < 2) {
        printUsage();
    }

    for (int i = 1; i < argc; ++i) {
        string input = argv[i];
        if (0 == input.compare(options::nobinary)) {
            conf.useBinaryClauses = false;
        } else if (0 == input.compare(options::intch_symm)) {
            conf.useMatrixDetection = true;
        } else if (0 == input.compare(options::onlybreakers)) {
            onlyPrintBreakers = true;
        } else if (0 == input.compare(options::generatorfile)) {
            printGeneratorFile = true;
        } else if (0 == input.compare(options::nosmall)) {
            conf.useShatterTranslation = true;
        } else if (0 == input.compare(options::norelaxed)) {
            conf.useFullTranslation = true;
        } else if (0 == input.compare(options::formlength)) {
            ++i;
            conf.symBreakingFormLength = std::stoi(argv[i]);
        } else if (0 == input.compare(options::steps_lim)) {
            ++i;
            conf.steps_lim = std::stoi(argv[i]);
        } else if (0 == input.compare(options::verbosity)) {
            ++i;
            conf.verbosity = std::stoi(argv[i]);
        } else if (0 == input.compare(options::help)) {
            printUsage();
        }
    }

    if (conf.verbosity > 1) {
        cout << "Options used:"
            << " " << options::formlength << " " << conf.symBreakingFormLength

            << " " << options::steps_lim << " " << conf.steps_lim

            << " " << options::verbosity << " " << conf.verbosity

            << " "
            << (conf.useMatrixDetection ? "" : options::intch_symm) << " "
            << (conf.useBinaryClauses ? "" : options::nobinary) << " "
            << (onlyPrintBreakers ? options::onlybreakers : "") << " "
            << (printGeneratorFile ? options::generatorfile : "") << " "
            << (conf.useShatterTranslation ? options::nosmall : "") << " "
            << (conf.useFullTranslation ? options::norelaxed : "") << " "
            << endl;
    }
}

int main(int argc, char *argv[])
{
    parseOptions(argc, argv);
    BID::BreakID breakid;
    breakid.set_useMatrixDetection(conf.useMatrixDetection);
    breakid.set_useBinaryClauses(conf.useBinaryClauses);
    breakid.set_useShatterTranslation(conf.useShatterTranslation);
    breakid.set_useFullTranslation(conf.useFullTranslation);
    breakid.set_symBreakingFormLength(conf.symBreakingFormLength);
    breakid.set_verbosity(conf.verbosity);


    string fname = argv[1];
    breakid.read_cnf(fname);

    if (conf.verbosity > 3) {
        breakid.print_graph();
    }

    if (conf.verbosity) {
        breakid.print_generators(std::cout);
    }

    if (conf.verbosity) {
        cout << "*** Detecting subgroups..." << endl;
    }
    breakid.detect_subgroups();

    if (conf.verbosity) {
        breakid.print_subgroups(cout);
    }
    breakid.break_symm();

    if (conf.verbosity) {
        breakid.print_symm_break_stats();
    }

    breakid.write_final_cnf(onlyPrintBreakers);

    if (printGeneratorFile) {
        string symFile = fname + ".sym";
        if (conf.verbosity) {
            cout
            << "*** Printing generators to file " + symFile
            << endl;
        }
        std::ofstream fp_out;
        fp_out.open(symFile, std::ios::out);
        breakid.print_generators(fp_out);
        fp_out.close();
    }

    return 0;
}
