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
#include <string>

using std::cout;
using std::string;
using std::endl;

#include "breakid.hpp"

bool printGeneratorFile = false;
bool onlyPrintBreakers = false;

//conf
bool conf_useMatrixDetection = false;
bool conf_useBinaryClauses = true;
bool conf_useShatterTranslation = false;
bool conf_useFullTranslation = false;
int  conf_symBreakingFormLength = 50;
uint32_t conf_verbosity = 1;
int64_t conf_timeLim = std::numeric_limits<int64_t>::max();

namespace options {
    string nointch = "-no-row";
    string nobinary = "-no-bin";
    string formlength = "-s";
    string verbosity = "-v";
    string timelim = "-t";
    string help = "-h";
    string nosmall = "-no-small";
    string norelaxed = "-no-relaxed";
    string onlybreakers = "-print-only-breakers";
    string generatorfile = "-with-generator-file";
}

void printUsage()
{
    BreakID bid;
    cout << "BreakID version " << bid.get_sha1_version() << endl;
    cout << "Usage: ./BreakID <cnf-file> "
              << "[" << options::help << "] "
              << "[" << options::nointch << "] "
              << "[" << options::nobinary << "] "
              << "[" << options::nosmall << "] "
              << "[" << options::norelaxed << "] "
              << "[" << options::formlength << " <nat>] "
              << "[" << options::timelim << " <nat>] "
              << "[" << options::verbosity << " <nat>] "
              << "[" << options::onlybreakers << "] "
              << "[" << options::generatorfile << "] "
              << "\n";

    cout
    << "\nOptions:\n"
    << options::help << "\n"
    << " Display this help message instead of running BreakID.\n"

    << options::nointch << "\n"
    << " Disable detection and breaking of row interchangeability.\n"

    << options::nobinary << "\n"
    << " Disable construction of additional binary symmetry breaking\n"
    << " clauses based on stabilizer subgroups.\n"

    << options::nosmall << "\n"
    << " Disable compact symmetry breaking encoding, use Shatter's\n"
    << " encoding instead.\n"

    << options::norelaxed << "\n"
    << " Disable relaxing constraints on auxiliary encoding\n"
    << " variables, use longer encoding instead.\n"

    << options::formlength << " <default: " << conf_symBreakingFormLength << ">\n"
    << " Limit the size of the constructed symmetry breaking\n"
    << " formula's, measured as the number of auxiliary variables\n"
    << " introduced. <-1> means no symmetry breaking.\n"

    << options::timelim << " <default: " << conf_timeLim << ">\n"
    << " Upper limit on computing steps spent, approximate measure for time.\n"

    << options::verbosity << " <default: " << conf_verbosity << ">\n"
    << " Verbosity of the output. <0> means no output other than the\n"
    << " CNF augmented with symmetry breaking clauses.\n"

    << options::onlybreakers << "\n"
    << " Do not print original theory, only the symmetry breaking"
    << " clauses.\n"

    << options::generatorfile << "\n"
    << " Return the generator symmetries as a <path-to-cnf>.sym file.\n"
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
            conf_useBinaryClauses = false;
        } else if (0 == input.compare(options::nointch)) {
            conf_useMatrixDetection = false;
        } else if (0 == input.compare(options::onlybreakers)) {
            onlyPrintBreakers = true;
        } else if (0 == input.compare(options::generatorfile)) {
            printGeneratorFile = true;
        } else if (0 == input.compare(options::nosmall)) {
            conf_useShatterTranslation = true;
        } else if (0 == input.compare(options::norelaxed)) {
            conf_useFullTranslation = true;
        } else if (0 == input.compare(options::formlength)) {
            ++i;
            conf_symBreakingFormLength = std::stoi(argv[i]);
        } else if (0 == input.compare(options::timelim)) {
            ++i;
            conf_timeLim = std::stoi(argv[i]);
        } else if (0 == input.compare(options::verbosity)) {
            ++i;
            conf_verbosity = std::stoi(argv[i]);
        } else if (0 == input.compare(options::help)) {
            printUsage();
        }
    }

    if (conf_verbosity > 1) {
        cout << "Options used:"
            << " " << options::formlength << " " << conf_symBreakingFormLength

            << " " << options::timelim << " " << conf_timeLim

            << " " << options::verbosity << " " << conf_verbosity

            << " "
            << (conf_useMatrixDetection ? "" : options::nointch) << " "
            << (conf_useBinaryClauses ? "" : options::nobinary) << " "
            << (onlyPrintBreakers ? options::onlybreakers : "") << " "
            << (printGeneratorFile ? options::generatorfile : "") << " "
            << (conf_useShatterTranslation ? options::nosmall : "") << " "
            << (conf_useFullTranslation ? options::norelaxed : "") << " "
            << endl;
    }
}

int main(int argc, char *argv[])
{
    parseOptions(argc, argv);
    BreakID breakid;
    breakid.set_useMatrixDetection(conf_useMatrixDetection);
    breakid.set_useBinaryClauses(conf_useBinaryClauses);
    breakid.set_useShatterTranslation(conf_useShatterTranslation);
    breakid.set_useFullTranslation(conf_useFullTranslation);
    breakid.set_symBreakingFormLength(conf_symBreakingFormLength);
    breakid.set_verbosity(conf_verbosity);


    string fname = argv[1];
    breakid.read_cnf(fname);

    if (conf_verbosity > 3) {
        breakid.print_graph();
    }

    if (conf_verbosity) {
        breakid.print_generators();
    }

    if (conf_verbosity) {
        cout << "*** Detecting subgroups..." << endl;
    }
    breakid.detect_subgroups();

    if (conf_verbosity) {
        breakid.print_subgroups();
    }

    breakid.clean_theory();
    breakid.break_symm();

    if (conf_verbosity) {
        breakid.print_symm_break_stats();
    }

    breakid.write_final_cnf(onlyPrintBreakers);

    if (printGeneratorFile) {
        string symFile = fname + ".sym";
        if (conf_verbosity) {
            cout
            << "*** Printing generators to file " + symFile
            << endl;
        }

        breakid.print_generators(symFile);
    }

    return 0;
}
