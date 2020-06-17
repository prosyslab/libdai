#include <dai/alldai.h>
using namespace dai;

#include <boost/lexical_cast.hpp>

#include <cassert>
#include <chrono>
#include <ctime>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "linenoise.h"

using namespace std;

static FactorGraph fg;
static PropertySet opts;
static BP bp;
static map<int, bool> clamps;

void initBP() {
    bp = BP(fg, opts);
    for (const auto& clamp : clamps) {
        int varIndex = clamp.first;
        bool varValue = clamp.second;
        bp.clamp(varIndex, varValue ? 1 : 0);
    }
    bp.init();
}

void queryVariable(stringstream &SS) {
  int varIndex;
  SS >> varIndex;
  clog << __LOGSTR__ << "Q " << varIndex << endl;

  // auto ans = bp.belief(fg.var(varIndex)).get(1);
  auto ans = bp.newBelief(varIndex);
  clog << __LOGSTR__ << "Returning " << ans << "." << endl;
  cout << ans << endl;
}

void queryFactor(stringstream &SS) {
  int factorIndex, valueIndex;
  SS >> factorIndex >> valueIndex;
  clog << __LOGSTR__ << "FQ " << factorIndex << " " << valueIndex << endl;

  auto ans = bp.beliefF(factorIndex).get(valueIndex);
  clog << __LOGSTR__ << "Returning " << ans << "." << endl;
  cout << ans << endl;
}

void runBP(stringstream &SS) {
  double tolerance;
  size_t minIters, maxIters, histLength;
  SS >> tolerance >> minIters >> maxIters >> histLength;
  clog << __LOGSTR__ << "BP " << tolerance << " " << minIters << " " << maxIters
       << " " << histLength << endl;

  double yetToConvergeFraction =
      bp.run(tolerance, minIters, maxIters, histLength);
  cout << yetToConvergeFraction << endl;
}

void clamp(stringstream &SS) {
  int varIndex;
  string varValueStr;
  SS >> varIndex >> varValueStr;
  clog << __LOGSTR__ << "O " << varIndex << " " << varValueStr << endl;
  if (varValueStr.empty()) {
    clog << "Invalid arguments" << endl;
    return;
  }
  if (varValueStr != "true" && varValueStr != "false") {
    clog << varIndex << "is " << varValueStr << " is an invalid boolean value"
         << endl;
    return;
  }
  bool varValue = (varValueStr == "true");
  clamps[varIndex] = varValue;
  initBP();
  cout << "O " << varIndex << " " << varValueStr << endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << __LOGSTR__ << "Insufficient number of arguments." << endl;
        cerr << __LOGSTR__ << "Insufficient number of arguments." << endl;
        return 1;
    }
    char *factorGraphFileName = argv[1];

    clog << __LOGSTR__ << "Hello!" << endl
                       << "Bingo compiled on " << __DATE__ << " at " << __TIME__ << "." << endl;
    fg.ReadFromFile(factorGraphFileName);
    clog << __LOGSTR__ << "Finished reading factor graph." << endl;

    opts.set("maxiter", static_cast<size_t>(10000000));
    opts.set("tol", Real(1e-6));
    opts.set("verb", static_cast<size_t>(1));
    opts.set("updates", string("SEQRND")); // "SEQRND", or "PARALL", or "SEQFIX", or "SEQRNDPAR"
    opts.set("logdomain", true);

    initBP();

    linenoiseHistoryLoad("history.txt");
    char *line;
    string CmdType;
    while ((line = linenoise("bingo> ")) != NULL) {

      linenoiseHistoryAdd(line);
      linenoiseHistorySave("history.txt");
      string Line(line);
      stringstream SS(Line);
      SS >> CmdType;
      clog << __LOGSTR__ << "Read command " << CmdType << endl;
      if (CmdType == "Q") {
        queryVariable(SS);
      } else if (CmdType == "FQ") {
        queryFactor(SS);
      } else if (CmdType == "BP") {
        runBP(SS);
      } else if (CmdType == "O") {
        clamp(SS);
      } else if (CmdType == "exit") {
        return 0;
      } else {
        cout << "Invalid command" << endl;
      }

      free(line);
    }
    clog << __LOGSTR__ << "Bye!" << endl;
    return 0;
}
