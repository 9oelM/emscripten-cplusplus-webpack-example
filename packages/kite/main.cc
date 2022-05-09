#include <iostream>
#include "proc.h"

using namespace std;

static string banner = "\
*******************************************************\n\
* Kite: An architecture simulator for five-stage      *\n\
* pipeline modeling of RISC-V instruction set         *\n\
* Developed by William J. Song                        *\n\
* School of Electrical Engineering, Yonsei University *\n\
* Version: 1.8                                        *\n\
*******************************************************\n\
";

int main(int argc, char **argv) {
    cout << banner << endl;

    if(argc != 2) {
        cerr << "Usage: " << argv[0] << " [program_code]" << endl;
        exit(1);
    }

    proc_t proc;            // Kite processor
    proc.init(argv[1]);     // Processor initialization
    proc.run();             // Processor runs.
    return 0;
}

