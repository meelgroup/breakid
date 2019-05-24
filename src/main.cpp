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

#include <string>
#include <iostream>

using std::cout;
using std::endl;

#include "breakid.hpp"
#include "config.hpp"

Config conf;
bool printGeneratorFile = false;
bool onlyPrintBreakers = false;

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
    BreakID bid(NULL);
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

    << options::formlength << " <default: " << conf.symBreakingFormLength << ">\n"
    << " Limit the size of the constructed symmetry breaking\n"
    << " formula's, measured as the number of auxiliary variables\n"
    << " introduced. <-1> means no symmetry breaking.\n"

    << options::timelim << " <default: " << conf.timeLim << ">\n"
    << " Upper limit on computing steps spent, approximate measure for time.\n"

    << options::verbosity << " <default: " << conf.verbosity << ">\n"
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
            conf.useBinaryClauses = false;
        } else if (0 == input.compare(options::nointch)) {
            conf.useMatrixDetection = false;
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
        } else if (0 == input.compare(options::timelim)) {
            ++i;
            conf.timeLim = std::stoi(argv[i]);
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

            << " " << options::timelim << " " << conf.timeLim

            << " " << options::verbosity << " " << conf.verbosity

            << " "
            << (conf.useMatrixDetection ? "" : options::nointch) << " "
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
    BreakID breakid(&conf);

    string fname = argv[1];
    breakid.read_cnf(fname);

    if (conf.verbosity > 3) {
        breakid.print_graph();
    }

    if (conf.verbosity) {
        breakid.print_generators();
    }

    breakid.detect_subgroups();

    if (conf.verbosity) {
        breakid.print_subgroups();
    }

    breakid.clean_theory();
    breakid.break_symm();

    if (conf.verbosity) {
        breakid.print_symm();
    }

    breakid.write_final_cnf(onlyPrintBreakers);

    if (printGeneratorFile) {
        breakid.print_generators(fname + ".sym");
    }

    return 0;
}
