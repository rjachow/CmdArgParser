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

TEST(CmdArgParserTestSuite, FlagsAreBundledTest)
{
    int argc;
    auto argv = createArgv("CmdArgParser.exe -fgi -j", argc);

    parser::CmdArgParser parser(argc, argv);

    ASSERT_TRUE(parser.declareFlag('f', "test1"));
    ASSERT_TRUE(parser.declareFlag('g', "test2"));
    ASSERT_TRUE(parser.declareFlag('i', "test3"));
    ASSERT_TRUE(parser.declareFlag('j', "test4"));

    ASSERT_TRUE(parser.parseArgs());

    ASSERT_TRUE(parser.hasFlag('f', "test1"));
    ASSERT_TRUE(parser.hasFlag('g', "test2"));
    ASSERT_TRUE(parser.hasFlag('i', "test3"));
    ASSERT_TRUE(parser.hasFlag('j', "test4"));

    destroyArgv(argv, argc);
}

TEST(CmdArgParserTestSuite, FlagsAreBundledWithOptionTest)
{
    int argc;
    auto argv = createArgv("CmdArgParser.exe -fgio", argc);

    parser::CmdArgParser parser(argc, argv);

    ASSERT_TRUE(parser.declareFlag('f', "test1"));
    ASSERT_TRUE(parser.declareFlag('g', "test2"));
    ASSERT_TRUE(parser.declareFlag('i', "test3"));
    ASSERT_TRUE(parser.declareOption('o', "test4"));

    ASSERT_FALSE(parser.parseArgs());

    destroyArgv(argv, argc);
}

TEST(CmdArgParserTestSuite, AlreadyUsedShortFlagTest)
{
    int argc;
    auto argv = createArgv("CmdArgParser.exe -f -f", argc);

    parser::CmdArgParser parser(argc, argv);

    ASSERT_TRUE(parser.declareFlag('f', "test1"));

    ASSERT_FALSE(parser.parseArgs());

    destroyArgv(argv, argc);
}

TEST(CmdArgParserTestSuite, AlreadyUsedShortOptionTest)
{
    int argc;
    auto argv = createArgv("CmdArgParser.exe -o option -o option", argc);

    parser::CmdArgParser parser(argc, argv);

    ASSERT_TRUE(parser.declareFlag('o', "test1"));

    ASSERT_FALSE(parser.parseArgs());

    destroyArgv(argv, argc);
}

TEST(CmdArgParserTestSuite, UndeclaredShortParameterTest)
{
    int argc;
    auto argv = createArgv("CmdArgParser.exe -u", argc);

    parser::CmdArgParser parser(argc, argv);

    ASSERT_TRUE(parser.declareFlag('f', "test1"));
    ASSERT_TRUE(parser.declareFlag('g', "test2"));
    ASSERT_TRUE(parser.declareFlag('i', "test3"));

    ASSERT_FALSE(parser.parseArgs());

    destroyArgv(argv, argc);
}

TEST(CmdArgParserTestSuite, UnknownArgumentNoDashTest)
{
    int argc;
    auto argv = createArgv("CmdArgParser.exe argument", argc);

    parser::CmdArgParser parser(argc, argv);

    ASSERT_FALSE(parser.parseArgs());

    destroyArgv(argv, argc);
}

TEST(CmdArgParserTestSuite, UnknownArgumentSingleDashTest)
{
    int argc;
    auto argv = createArgv("CmdArgParser.exe -", argc);

    parser::CmdArgParser parser(argc, argv);

    ASSERT_FALSE(parser.parseArgs());

    destroyArgv(argv, argc);
}

TEST(CmdArgParserTestSuite, UnknownArgumentDoubleDashTest)
{
    int argc;
    auto argv = createArgv("CmdArgParser.exe --", argc);

    parser::CmdArgParser parser(argc, argv);

    ASSERT_FALSE(parser.parseArgs());

    destroyArgv(argv, argc);
}

TEST(CmdArgParserTestSuite, DeclarationDuplicationTest)
{
    int argc;
    auto argv = createArgv("CmdArgParser.exe -h", argc);

    parser::CmdArgParser parser(argc, argv);

    ASSERT_FALSE(parser.declareFlag('h', "test1"));
    ASSERT_FALSE(parser.declareFlag('t', "help"));
    ASSERT_FALSE(parser.declareOption('h', "test2"));
    ASSERT_FALSE(parser.declareOption('t', "help"));

    ASSERT_TRUE(parser.declareFlag('f', "testf"));
    ASSERT_TRUE(parser.declareOption('o', "testo"));

    ASSERT_FALSE(parser.declareFlag('f', "test3"));
    ASSERT_FALSE(parser.declareFlag('t', "testf"));
    ASSERT_FALSE(parser.declareOption('f', "test4"));
    ASSERT_FALSE(parser.declareOption('t', "testf"));

    ASSERT_FALSE(parser.declareFlag('o', "test5"));
    ASSERT_FALSE(parser.declareFlag('t', "testo"));
    ASSERT_FALSE(parser.declareOption('o', "test6"));
    ASSERT_FALSE(parser.declareOption('t', "testo"));

    destroyArgv(argv, argc);
}