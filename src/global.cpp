#include "global.hpp"
#include "limits.h"

using namespace std;

uint nVars = 0;
std::vector<uint> fixedLits;
std::string inputSymFile = "";
time_t startTime;

// OPTIONS:
bool useMatrixDetection = true;
bool useBinaryClauses = true;
bool onlyPrintBreakers = false;
bool printGeneratorFile = false;
bool useShatterTranslation = false;
bool useFullTranslation = false;
int symBreakingFormLength = 50;
bool aspinput = false;
uint verbosity = 1;
int timeLim = INT_MAX;

size_t _getHash(const std::vector<uint>& xs) {
  size_t seed = xs.size();
  for (auto x : xs) {
    seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
  return seed;
}

size_t _getHash(const std::vector<int>& xs) {
  size_t seed = xs.size();
  for (auto x : xs) {
    seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
  return seed;
}

int timeLeft() {
  time_t now;
  time(&now);
  return timeLim - difftime(now, startTime);
}

bool timeLimitPassed() {
  return timeLeft() <= 0;
}

void gracefulError(string str) {
  std::cerr << str << "\nExiting..." << endl;
  exit(1);
}
