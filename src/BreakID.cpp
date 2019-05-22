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

void addInputSym(sptr<Group> group)
{
    if (inputSymFile == "") {
        return; // no input symmetry
    }
    if (verbosity > 0) {
        cout << "*** Reading input symmetry from: " << inputSymFile
                  << endl;
    }

    std::ifstream file(inputSymFile);
    if (!file) {
        gracefulError("No input symmetry file found.");
    }

    string line;
    std::vector<uint32_t> cycle;
    while (getline(file, line)) {
        if (line.front() == '(') { // this is a symmetry generator line
            if (verbosity > 1) {
                cout << "**** adding input symmetry generator"
                          << endl;
            }

            sptr<Permutation> perm = std::make_shared<Permutation>();

            stringstream sym(line);
            string cycle_str;

            getline(sym, cycle_str, '('); // skip first empty line
            while (std::getline(sym, cycle_str, '(')) {
                stringstream cycle_ss(cycle_str);
                cycle.clear();
                int l = 0;
                while (cycle_ss >> l) {
                    cycle.push_back(encode(l));
                }
                perm->addCycle(cycle);
            }
            group->add(perm);
            perm->print(std::cerr);
        } else if (line.front() ==
                   'r') { // this is an interchangeability matrix block
            if (verbosity > 1) {
                cout << "**** adding input symmetry matrix" << endl;
            }
            uint32_t nbRows;
            uint32_t nbColumns;
            char tmp;
            istringstream iss(line);
            for (uint32_t i = 0; i < 4; ++i) {
                iss >> tmp;
            } // skip "rows"
            iss >> nbRows;
            for (uint32_t i = 0; i < 7; ++i) {
                iss >> tmp;
            } // skip "columns"
            iss >> nbColumns;
            sptr<Matrix> mat = std::make_shared<Matrix>();

            for (uint32_t r = 0; r < nbRows; ++r) {
                std::vector<uint32_t> *newRow = new std::vector<uint32_t>(nbColumns);
                getline(file, line);
                istringstream iss_row(line);
                int tmp;
                for (uint32_t c = 0; c < nbColumns; ++c) {
                    iss_row >> tmp;
                    (*newRow)[c] = encode(tmp);
                }
                mat->add(newRow);
            }
            group->addMatrix(mat);
        } else {
            gracefulError("Unexpected line in input symmetry file: " + line);
        }
    }
}

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
string symmetryinput = "-addsym";
} // namespace options

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
              << "[" << options::symmetryinput << "<file with symmetry info>] "
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

    << options::formlength << " <default: " << symBreakingFormLength << ">\n"
    << " Limit the size of the constructed symmetry breaking\n"
    << " formula's, measured as the number of auxiliary variables\n"
    << " introduced. <-1> means no symmetry breaking.\n"

    << options::timelim << " <default: " << timeLim << ">\n"
    << " Upper limit on time spent by Saucy detecting symmetry\n"
    << " measured in seconds.\n"

    << options::verbosity << " <default: " << verbosity << ">\n"
    << " Verbosity of the output. <0> means no output other than the\n"
    << " CNF augmented with symmetry breaking clauses.\n"

    << options::onlybreakers << "\n"
    << " Do not print original theory, only the symmetry breaking"
    << " clauses.\n"

    << options::generatorfile << "\n"
    << " Return the generator symmetries as a <path-to-cnf>.sym file.\n"

    << options::symmetryinput << " <default: none>\n"
    << " Pass a file with symmetry generators or row-interchangeable\n"
    << " matrices to use as additional symmetry information. Same\n"
    << " format as BreakID's output by " << options::generatorfile << ".\n"
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
            useBinaryClauses = false;
        } else if (0 == input.compare(options::nointch)) {
            useMatrixDetection = false;
        } else if (0 == input.compare(options::onlybreakers)) {
            onlyPrintBreakers = true;
        } else if (0 == input.compare(options::generatorfile)) {
            printGeneratorFile = true;
        } else if (0 == input.compare(options::nosmall)) {
            useShatterTranslation = true;
        } else if (0 == input.compare(options::norelaxed)) {
            useFullTranslation = true;
        } else if (0 == input.compare(options::formlength)) {
            ++i;
            symBreakingFormLength = std::stoi(argv[i]);
        } else if (0 == input.compare(options::timelim)) {
            ++i;
            timeLim = std::stoi(argv[i]);
        } else if (0 == input.compare(options::verbosity)) {
            ++i;
            verbosity = std::stoi(argv[i]);
        } else if (0 == input.compare(options::help)) {
            printUsage();
        } else if (0 == input.compare(options::symmetryinput)) {
            ++i;
            inputSymFile = argv[i];
        }
    }

    if (verbosity > 1) {
        cout << "Options used: " << options::formlength << " "
                  << symBreakingFormLength << " " << options::timelim << " "
                  << timeLim << " " << options::verbosity << " " << verbosity
                  << " " << (useMatrixDetection ? "" : options::nointch) << " "
                  << (useBinaryClauses ? "" : options::nobinary) << " "
                  << (onlyPrintBreakers ? options::onlybreakers : "") << " "
                  << (printGeneratorFile ? options::generatorfile : "") << " "
                  << (inputSymFile != "" ? options::symmetryinput : " ") << " "
                  << (useShatterTranslation ? options::nosmall : "") << " "
                  << (useFullTranslation ? options::norelaxed : "") << " "
                  << endl;
    }
}

// ==== main
int main(int argc, char *argv[])
{
    parseOptions(argc, argv);

    time(&startTime);
    string filename_ = argv[1];

    sptr<Specification> theory;
    theory = make_shared<CNF>(filename_);

    if (verbosity > 3) {
        theory->getGraph()->print();
    }

    if (verbosity > 0) {
        cout << "**** symmetry generators detected: "
                  << theory->getGroup()->getSize() << endl;
        if (verbosity > 2) {
            theory->getGroup()->print(cout);
        }
    }

    addInputSym(theory->getGroup());

    if (verbosity > 0) {
        cout << "*** Detecting subgroups..." << endl;
    }
    vector<sptr<Group> > subgroups;
    theory->getGroup()->getDisjointGenerators(subgroups);
    if (verbosity > 0) {
        cout << "**** subgroups detected: " << subgroups.size()
                  << endl;
    }

    if (verbosity > 1) {
        for (auto grp : subgroups) {
            cout << "group size: " << grp->getSize()
                      << " support: " << grp->getSupportSize() << endl;
            if (verbosity > 2) {
                grp->print(cout);
            }
        }
    }

    theory->cleanUp(); // improve some memory overhead

    uint32_t totalNbMatrices = 0;
    uint32_t totalNbRowSwaps = 0;

    Breaker brkr(theory);
    for (auto grp : subgroups) {
        if (grp->getSize() > 1 && useMatrixDetection) {
            if (verbosity > 1) {
                cout << "*** Detecting row interchangeability..."
                          << endl;
            }
            theory->setSubTheory(grp);
            grp->addMatrices();
            totalNbMatrices += grp->getNbMatrices();
            totalNbRowSwaps += grp->getNbRowSwaps();
        }
        if (symBreakingFormLength > -1) {
            if (verbosity > 1) {
                cout << "*** Constructing symmetry breaking formula..."
                          << endl;
            }
            grp->addBreakingClausesTo(brkr);
        } // else no symmetry breaking formulas are needed :)
        grp.reset();
    }

    if (verbosity > 0) {
        cout << "**** matrices detected: " << totalNbMatrices << endl;
        cout << "**** row swaps detected: " << totalNbRowSwaps
                  << endl;
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
        cout
            << "*** Printing resulting theory with symmetry breaking clauses..."
            << endl;
    }

    brkr.print(filename_);

    if (printGeneratorFile) {
        string symFile = filename_ + ".sym";
        if (verbosity > 0) {
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
