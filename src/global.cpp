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
