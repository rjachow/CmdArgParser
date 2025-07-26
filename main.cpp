#include "src/CmdArgParser.hpp"

#include <iostream>

int main(int argc, char** argv) {
    parser::CmdArgParser parser(argc, argv, "CmdArgParser Example Program");

    parser.declareFlag('f', "flag", false, "Desc");
    parser.declareOption('o', "option", false, "Desc");

    parser.parseArgs();

    return 0;
}