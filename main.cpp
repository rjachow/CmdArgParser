#include "src/CmdArgParser.hpp"

#include <iostream>

int main(int argc, char** argv) {
    parser::CmdArgParser parser(argc, argv, "CmdArgParser Example Program");

    parser.declareFlag('f', "flag", true, "Desc. Long Description.dsdfjdshfjdsfdsfhshdjfhjisdfjdjfiwijdfnjiwdjfjwidjfjdwnfwijdfwijdfjwjdfnjjdwi");
    parser.declareOption('o', "option", true, "Desc");

    parser.parseArgs();

    return 0;
}