#include "src/CmdArgParser.hpp"

#include <iostream>

int main(int argc, char** argv) {
    CmdArgParser parser(argc, argv, "CmdArgParser Example Program");

    parser.declareFlag('h', "dddd", false, "Desc");

    return 0;
}