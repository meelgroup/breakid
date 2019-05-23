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

#include <cstdlib>
#include <string>
#include <fstream>
#include <iterator>
#include <sstream>

#include "Algebraic.hpp"
#include "Breaking.hpp"
#include "GitSHA1.h"
#include "Graph.hpp"
#include "Theory.hpp"
#include "global.hpp"

using std::cout;
using std::endl;
using std::string;
using std::ofstream;
using std::stringstream;
using std::istringstream;
using std::make_shared;
using std::vector;

namespace options {
    // option strings:
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

Config conf;

void printUsage()
{
    cout << "BreakID version " << BreakID::get_version_sha1() << endl;
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
            conf.onlyPrintBreakers = true;
        } else if (0 == input.compare(options::generatorfile)) {
            conf.printGeneratorFile = true;
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
            << (conf.onlyPrintBreakers ? options::onlybreakers : "") << " "
            << (conf.printGeneratorFile ? options::generatorfile : "") << " "
            << (conf.useShatterTranslation ? options::nosmall : "") << " "
            << (conf.useFullTranslation ? options::norelaxed : "") << " "
            << endl;
    }
}

int main(int argc, char *argv[])
{
    parseOptions(argc, argv);
    string filename_ = argv[1];

    sptr<Specification> theory;
    theory = make_shared<CNF>(filename_, &conf);

    if (conf.verbosity > 3) {
        theory->getGraph()->print();
    }

    if (conf.verbosity) {
        cout << "**** symmetry generators detected: "
                  << theory->getGroup()->getSize() << endl;
        if (conf.verbosity > 2) {
            theory->getGroup()->print(cout);
        }
    }

    if (conf.verbosity) {
        cout << "*** Detecting subgroups..." << endl;
    }
    vector<sptr<Group> > subgroups;
    theory->getGroup()->getDisjointGenerators(subgroups);
    if (conf.verbosity) {
        cout << "**** subgroups detected: " << subgroups.size()
                  << endl;
    }

    if (conf.verbosity > 1) {
        for (auto grp : subgroups) {
            cout << "group size: " << grp->getSize()
                      << " support: " << grp->getSupportSize() << endl;
            if (conf.verbosity > 2) {
                grp->print(cout);
            }
        }
    }

    theory->cleanUp(); // improve some memory overhead

    uint32_t totalNbMatrices = 0;
    uint32_t totalNbRowSwaps = 0;

    Breaker brkr(theory, &conf);
    for (auto grp : subgroups) {
        if (grp->getSize() > 1 && conf.useMatrixDetection) {
            if (conf.verbosity > 1) {
                cout << "*** Detecting row interchangeability..." << endl;
            }
            theory->setSubTheory(grp);
            grp->addMatrices();
            totalNbMatrices += grp->getNbMatrices();
            totalNbRowSwaps += grp->getNbRowSwaps();
        }
        if (conf.symBreakingFormLength > -1) {
            if (conf.verbosity > 1) {
                cout << "*** Constructing symmetry breaking formula..." << endl;
            }
            grp->addBreakingClausesTo(brkr);
        } // else no symmetry breaking formulas are needed :)
        grp.reset();
    }

    if (conf.verbosity) {
        cout << "**** matrices detected: " << totalNbMatrices << endl;
        cout << "**** row swaps detected: " << totalNbRowSwaps << endl;

        cout << "**** extra binary symmetry breaking clauses added: "
                  << brkr.getNbBinClauses() << "\n";

        cout << "**** regular symmetry breaking clauses added: "
                  << brkr.getNbRegClauses() << "\n";

        cout << "**** row interchangeability breaking clauses added: "
                  << brkr.getNbRowClauses() << "\n";

        cout << "**** total symmetry breaking clauses added: "
                  << brkr.getAddedNbClauses() << "\n";

        cout << "**** auxiliary variables introduced: "
                  << brkr.getAuxiliaryNbVars() << "\n";

        cout << "*** Printing resulting theory with symm breaking clauses..."
        << endl;
    }

    brkr.print(filename_);

    if (conf.printGeneratorFile) {
        string symFile = filename_ + ".sym";
        if (conf.verbosity) {
            cout << "*** Printing generators to file " + symFile
                      << endl;
        }
        ofstream fp_out;
        fp_out.open(symFile, std::ios::out);
        for (auto grp : subgroups) {
            grp->print(fp_out);
        }
        fp_out.close();
    }

    return 0;
}
