#include <fstream>
#include <sstream>
#include <iterator>
#include <cstring>
#include <stdlib.h>

#include "Graph.hpp"
#include "global.hpp"
#include "Algebraic.hpp"
#include "Theory.hpp"
#include "Breaking.hpp"
#include "GitSHA1.h"

using namespace std;

void setFixedLits(std::string& filename) {
  if (verbosity > 0) {
    std::clog << "*** Reading fixed variables: " << filename << std::endl;
  }

  ifstream file(filename);
  if (!file) {
    gracefulError("No fixed variables file found.");
  }
  
  fixedLits.clear();
  string line;
  while (getline(file, line)) {
    istringstream iss(line);
    int l;
    while (iss >> l) {
      fixedLits.push_back(encode(l));
      fixedLits.push_back(encode(-l));
    }
  }
}

void addInputSym(sptr<Group> group) {
  if(inputSymFile == ""){
  	return; // no input symmetry
  }
  if (verbosity > 0) {
    std::clog << "*** Reading input symmetry from: " << inputSymFile << std::endl;
  }
  
  ifstream file(inputSymFile);
  if (!file) {
    gracefulError("No input symmetry file found.");
  }
  
  string line;
  std::vector<uint> cycle;
  while (getline(file, line)) {
    if (line.front() == '(') { // this is a symmetry generator line    
      if(verbosity>1){
        std::clog << "**** adding input symmetry generator" << std::endl;
      }

      sptr<Permutation> perm = std::make_shared<Permutation>();
      
      stringstream sym(line);
      string cycle_str;

      getline(sym, cycle_str, '('); // skip first empty line
      while(std::getline(sym, cycle_str, '(')){
        stringstream cycle_ss(cycle_str);
        cycle.clear();
        int l=0;
        while(cycle_ss >> l){
          cycle.push_back(encode(l));
        }
        perm->addCycle(cycle);
      }
      group->add(perm);
      perm->print(std::cerr);
    }else if (line.front() == 'r'){ // this is an interchangeability matrix block
      if(verbosity>1){
        std::clog << "**** adding input symmetry matrix" << std::endl;
      }
      uint nbRows;
      uint nbColumns;
      char tmp;
      istringstream iss(line);
      for(uint i=0; i<4; ++i){ iss >> tmp; } // skip "rows"
      iss >> nbRows;
      for(uint i=0; i<7; ++i){ iss >> tmp; } // skip "columns"
      iss >> nbColumns;
      sptr<Matrix> mat = std::make_shared<Matrix>();
      
      for(uint r=0; r<nbRows; ++r){
        std::vector<uint>* newRow = new std::vector<uint>(nbColumns);
        getline(file, line);
        istringstream iss_row(line);
        int tmp;
        for(uint c=0; c<nbColumns; ++c){
          iss_row >> tmp;
          (*newRow)[c]=encode(tmp);
        }
        mat->add(newRow);
      }
      group->addMatrix(mat);
    }else{
      gracefulError("Unexpected line in input symmetry file: "+line);
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
  string fixedvars = "-fixed";
  string onlybreakers = "-print-only-breakers";
  string generatorfile = "-with-generator-file";
  string symmetryinput = "-addsym";
  string aspinput = "-asp";
}

void printUsage() {
  std::clog << "BreakID version " << BreakID::get_version_sha1() << std::endl;
  std::clog << "Usage: ./BreakID <cnf-file> " <<
          "[" << options::help << "] " <<
          "[" << options::nointch << "] " <<
          "[" << options::nobinary << "] " <<
          "[" << options::nosmall << "] " <<
          "[" << options::norelaxed << "] " <<
          "[" << options::formlength << " <nat>] " <<
          "[" << options::timelim << " <nat>] " <<
          "[" << options::verbosity << " <nat>] " <<
          "[" << options::fixedvars << " <file with fixed vars>] " <<
          "[" << options::onlybreakers << "] " <<
          "[" << options::generatorfile << "] " <<
          "[" << options::symmetryinput << "<file with symmetry info>] " <<
          "[" << options::aspinput << "] " <<
          "\n";
  std::clog << "\nOptions:\n";
  std::clog << options::help << "\n  ";
  std::clog << "Display this help message instead of running BreakID.\n";
  std::clog << options::nointch << "\n  ";
  std::clog << "Disable detection and breaking of row interchangeability.\n";
  std::clog << options::nobinary << "\n  ";
  std::clog << "Disable construction of additional binary symmetry breaking clauses based on stabilizer subgroups.\n";
  std::clog << options::nosmall << "\n  ";
  std::clog << "Disable compact symmetry breaking encoding, use Shatter's encoding instead.\n";
  std::clog << options::norelaxed << "\n  ";
  std::clog << "Disable relaxing constraints on auxiliary encoding variables, use longer encoding instead.\n";
  std::clog << options::formlength << " <default: " << symBreakingFormLength << ">\n  ";
  std::clog << "Limit the size of the constructed symmetry breaking formula's, measured as the number of auxiliary variables introduced. <-1> means no symmetry breaking.\n";
  std::clog << options::timelim << " <default: " << timeLim << ">\n  ";
  std::clog << "Upper limit on time spent by Saucy detecting symmetry measured in seconds.\n";
  std::clog << options::verbosity << " <default: " << verbosity << ">\n  ";
  std::clog << "Verbosity of the output. <0> means no output other than the CNF augmented with symmetry breaking clauses.\n";
  std::clog << options::fixedvars << " <default: none>\n  ";
  std::clog << "File with a list of variables that should be fixed, separated by whitespace.\n";
  std::clog << options::onlybreakers << "\n  ";
  std::clog << "Do not print original theory, only the symmetry breaking clauses.\n";
  std::clog << options::generatorfile << "\n  ";
  std::clog << "Return the generator symmetries as a <path-to-cnf>.sym file.\n";
  std::clog << options::symmetryinput << " <default: none>\n  ";
  std::clog << "Pass a file with symmetry generators or row-interchangeable matrices to use as additional symmetry information. Same format as BreakID's output by " << options::generatorfile << ".\n";
  std::clog << options::aspinput << "\n  ";
  std::clog << "Parse input in the smodels-lparse intermediate format instead of DIMACS.\n";
  gracefulError("");
}

void parseOptions(int argc, char *argv[]) {
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
      symBreakingFormLength = stoi(argv[i]);
    } else if (0 == input.compare(options::timelim)) {
      ++i;
      timeLim = stoi(argv[i]);
    } else if (0 == input.compare(options::verbosity)) {
      ++i;
      verbosity = stoi(argv[i]);
    } else if (0 == input.compare(options::help)) {
      printUsage();
    } else if (0 == input.compare(options::fixedvars)) {
      ++i;
      string filename = argv[i];
      setFixedLits(filename);
    } else if (0 == input.compare(options::aspinput)) {
      aspinput = true;
    } else if (0 == input.compare(options::symmetryinput)) {
      ++i;
      inputSymFile = argv[i];
    }
  }

  if (verbosity > 1) {
    std::clog << "Options used: " <<
            options::formlength << " " << symBreakingFormLength << " " <<
            options::timelim << " " << timeLim << " " <<
            options::verbosity << " " << verbosity << " " <<
            (useMatrixDetection ? "" : options::nointch) << " " <<
            (useBinaryClauses ? "" : options::nobinary) << " " <<
            (onlyPrintBreakers ? options::onlybreakers : "") << " " <<
            (printGeneratorFile ? options::generatorfile : "") << " " <<
            (inputSymFile != "" ? options::symmetryinput : " ") << " " <<
            (useShatterTranslation ? options::nosmall : "") << " " <<
            (useFullTranslation ? options::norelaxed : "") << " " <<
            (fixedLits.size()==0 ? "" : options::fixedvars) << " " <<
            (aspinput ? options::aspinput : " ") << " " <<
            std::endl;
  }
}

// ==== main

void checkOptionsCompatible(){
  if(aspinput && onlyPrintBreakers){
    std::stringstream ss;
    ss << "Options " << options::aspinput << " and " << options::onlybreakers << " are incompatible since asp output is intertwined with extra clauses.\n";
    gracefulError(ss.str());
  }
}

void mktmpfile(string& filename_) {
  /* Get TMPDIR env variable or fall back to /tmp/ */
  std::stringstream ss;
  auto tmpdir = getenv("TMPDIR");
  if (tmpdir == NULL){
    ss << "/tmp";
  } else{
    ss << tmpdir;
  }
  ss << "/" << "breakid.tempfile.";
  srand ( time(NULL) );
  //REALLY RANDOM TMPFILE
  ss << rand()<< rand()<< rand();
  ss << "XXXXXXX";

  auto tmp = std::string(ss.str());
  char * filename = new char[tmp.size() + 1];
  std::copy(tmp.begin(), tmp.end(), filename);
  filename[tmp.size()] = '\0'; // don't forget the terminating 0

  filename_ = mktemp(filename);

  // don't forget to free the string after finished using it
  delete[] filename;

  if (verbosity > 0) {
    std::clog << "*** Reading stdin. Using temporary file: " << filename_ << '\n';
  }
  std::ofstream out(filename_);

  std::string line;
  while (std::getline(std::cin, line)) {
    out << line << std::endl;
  }
  out.close();
}

int main(int argc, char *argv[]) {
  parseOptions(argc, argv);
  checkOptionsCompatible();

  time(&startTime);
  string filename_ = argv[1];

  bool tmpfile = (filename_.size() == 1 && filename_.front() == '-');

  if (tmpfile) {
    mktmpfile(filename_);
  }

  sptr<Specification> theory;
  if(aspinput){
    theory = make_shared<LogicProgram>(filename_);
  } else{
    theory = make_shared<CNF>(filename_);
  }

  if (verbosity > 3) {
    theory->getGraph()->print();
  }

  if (verbosity > 0) {
    std::clog << "**** symmetry generators detected: " << theory->getGroup()->getSize() << std::endl;
    if (verbosity > 2) {
      theory->getGroup()->print(std::clog);
    }
  }
  
  addInputSym(theory->getGroup());
  
  if(verbosity>0){
    std::clog << "*** Detecting subgroups..." << std::endl;
  }
  vector<sptr<Group> > subgroups;
  theory->getGroup()->getDisjointGenerators(subgroups);
  if (verbosity > 0) {
    std::clog << "**** subgroups detected: " << subgroups.size() << std::endl;
  }

  if (verbosity > 1) {
    for (auto grp : subgroups) {
      std::clog << "group size: " << grp->getSize() << " support: " << grp->getSupportSize() << std::endl;
      if (verbosity > 2) {
        grp->print(std::clog);
      }
    }
  }
  
  theory->cleanUp(); // improve some memory overhead
  
  uint totalNbMatrices = 0;
  uint totalNbRowSwaps = 0;
  
  Breaker brkr(theory);
  for (auto grp : subgroups) {
    if (grp->getSize() > 1 && useMatrixDetection) {
      if (verbosity > 1) {
        std::clog << "*** Detecting row interchangeability..." << std::endl;
      }
      theory->setSubTheory(grp);
      grp->addMatrices();
      totalNbMatrices += grp->getNbMatrices();
      totalNbRowSwaps += grp->getNbRowSwaps();
    }
    if(symBreakingFormLength > -1){
        if (verbosity > 1) {
            std::clog << "*** Constructing symmetry breaking formula..." << std::endl;
        }
        grp->addBreakingClausesTo(brkr);  
    }// else no symmetry breaking formulas are needed :)
    grp.reset();
  }
  
  if (verbosity > 0) {
    std::clog << "**** matrices detected: " << totalNbMatrices << std::endl;
    std::clog << "**** row swaps detected: " << totalNbRowSwaps << std::endl;
    std::clog << "**** extra binary symmetry breaking clauses added: " << brkr.getNbBinClauses() << "\n";
    std::clog << "**** regular symmetry breaking clauses added: " << brkr.getNbRegClauses() << "\n";
    std::clog << "**** row interchangeability breaking clauses added: " << brkr.getNbRowClauses() << "\n";
    std::clog << "**** total symmetry breaking clauses added: " << brkr.getAddedNbClauses() << "\n";
    std::clog << "**** auxiliary variables introduced: " << brkr.getAuxiliaryNbVars() << "\n";
    std::clog << "*** Printing resulting theory with symmetry breaking clauses..." << std::endl;
  }

  brkr.print(filename_);

  if(printGeneratorFile){
    string symFile = filename_ + ".sym";
    if (verbosity > 0) {
      std::clog << "*** Printing generators to file "+symFile << std::endl;
    }
    ofstream fp_out;
    fp_out.open(symFile, ios::out);
    for (auto grp : subgroups) {
      grp->print(fp_out);
    }
    fp_out.close();
  }

  if(tmpfile){
    if(verbosity>0){
      std::clog << "*** removing temporary file\n";
    }
    std::remove(filename_.c_str());
  }

  return 0;
}
