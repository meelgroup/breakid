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

int main(int argc, char *argv[])
{
    BreakID breakid;
    breakid.set_useMatrixDetection(false);
//     breakid.set_useBinaryClauses(conf_useBinaryClauses);
//     breakid.set_useShatterTranslation(conf_useShatterTranslation);
//     breakid.set_useFullTranslation(conf_useFullTranslation);
//     breakid.set_symBreakingFormLength(conf_symBreakingFormLength);

    bool conf_verbosity = 3;
    breakid.set_verbosity(conf_verbosity);
    breakid.start_dynamic_cnf(10, 10);

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

    breakid.write_final_cnf(true);

    if (true) {
        string symFile = "test.sym";
        if (conf_verbosity) {
            cout
            << "*** Printing generators to file " + symFile
            << endl;
        }

        breakid.print_generators(symFile);
    }

    return 0;
}
