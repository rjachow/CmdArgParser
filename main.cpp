#include "src/CmdArgParser.hpp"

#include <iostream>

int main(int argc, char** argv) {
    CmdArgParser parser(argc, argv, "CmdArgParser Example Program");

    parser.declareOption('h', "lala", false, "Desc");

    return 0;
}