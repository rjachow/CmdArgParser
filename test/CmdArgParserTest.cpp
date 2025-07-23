#include "gtest/gtest.h"

#include "CmdArgParser.hpp"

#include <cstring>
#include <sstream>

char** createArgv(const std::string& argsStr, int& argcOut)
{
    std::stringstream argsStream(argsStr);
    std::vector<std::string> argsVec;
    std::string arg;
    while (argsStream >> arg)
    {
        argsVec.push_back(arg);
    }

    argcOut = static_cast<int>(argsVec.size());
    char **argvOut = new char *[argcOut];

    for (int i = 0; i < argcOut; ++i)
    {
        argvOut[i] = new char[argsVec[i].length() + 1];
        std::strcpy(argvOut[i], argsVec[i].c_str());
    }
    return argvOut;
}

void destroyArgv(char **argv, int argc)
{
    if (argv == nullptr)
    {
        return;
    }

    for (int i = 0; i < argc; ++i)
    {
        delete[] argv[i];
    }

    delete[] argv;
}

TEST(CmdArgParserTestSuite, DeclarationDuplicationTest)
{
    int argc;
    auto argv = createArgv("CmdArgParser -h", argc);

    CmdArgParser parser(argc, argv);

    ASSERT_FALSE(parser.declareFlag('h', "test1"));
    ASSERT_FALSE(parser.declareFlag('t', "help"));
    ASSERT_FALSE(parser.declareOption('h', "test3"));
    ASSERT_FALSE(parser.declareOption('t', "help"));

    destroyArgv(argv, argc);
}