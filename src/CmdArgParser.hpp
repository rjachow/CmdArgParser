#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <iostream>
#include <vector>

class CmdArgParser
{
public:
    CmdArgParser(int argc, char **argv, std::string programDescription = "")
        : mArgc(argc), mArgv(argv), mProgramDescription(std::move(programDescription))
    {
        declareFlag('h', "help", false, "Display this help message");

        parseArgs();
    }

    // Declarators
    void declareOption(const char shortName, const std::string &longName, bool required = false, std::string description = "")
    {
        if (checkDeclarationPossibility(shortName, longName))
        {
            Parameter param{shortName, longName, required, description};
            mDeclaredOptionsShortsMap.emplace(shortName, param);
            mDeclaredOptionsLongsMap.emplace(longName, param);
            mDeclaredOptions.insert(param);
        }
    }

    void declareFlag(const char shortName, const std::string &longName, bool required = false, std::string description = "")
    {
        if (checkDeclarationPossibility(shortName, longName))
        {
            Parameter param{shortName, longName, required, description};
            mDeclaredFlagsShortsMap.emplace(shortName, param);
            mDeclaredFlagsLongsMap.emplace(longName, param);
            mDeclaredFlags.insert(param);
        }
    }

    // Checkers
    bool hasOption(const char shortName, const std::string &longName) const
    {
        return mOptions.contains({shortName, longName});
    }

    std::optional<std::string> getOptionValue(const char shortName, const std::string &longName) const
    {
        const auto it = mOptions.find({shortName, longName});
        if (it != mOptions.end())
        {
            return it->second.c_str();
        }
        return std::nullopt;
    }

    bool hasFlag(char shortName, std::string longName) const
    {
        return mFlags.contains({shortName, longName});
    }

private:
    bool checkDeclarationPossibility(const char shortName, const std::string &longName)
    {
        if (mDeclaredFlagsShortsMap.contains(shortName) || mDeclaredFlagsLongsMap.contains(longName))
        {
            std::cout << "Unable to declare: -" << shortName << ", --" << longName << ". Already declared in flags.\n"
                      << "Declared parameters:\n";
            printDeclared();
            return false;
        }

        if (mDeclaredOptionsShortsMap.contains(shortName) || mDeclaredOptionsLongsMap.contains(longName))
        {
            std::cout << "Unable to declare: -" << shortName << ", --" << longName << ". Already declared in options.\n"
                      << "Declared parameters:\n";
            printDeclared();
            return false;
        }

        return true;
    }

    void parseArgs()
    {
        std::vector<std::string> cmdArgs;
        for (int i = 1; i < mArgc; ++i)
        {
            cmdArgs.push_back(mArgv[i]);
        }

        for (size_t i = 0; i < cmdArgs.size(); ++i)
        {
            const auto &arg = cmdArgs[i];
            if (arg == "-h" || arg == "--help")
            {
                printHelp();
                return;
            }
            else if (arg.starts_with("--") && arg.size() > 2)
            {
            }
            else if (arg.starts_with("-") && arg.size() > 1)
            {
                if (arg.size() > 2)
                {
                    // Many flags is bundled under one '-'
                    for (size_t si = 1; si < arg.size(); ++si)
                    {
                        if (mDeclaredOptionsShortsMap.contains(arg[si]))
                        {
                            std::cout << "Cannot bundle options. Each option needs a separate usage. Option: " << arg[si] << " bundled in phrase: " << arg << "\n";
                            return;
                        }

                        if (!mDeclaredFlagsShortsMap.contains(arg[si]))
                        {
                            std::cout << "Unknown flag: " << arg[si] << "\n";
                            return;
                        }

                        mFlags.insert(mDeclaredFlagsShortsMap.at(arg[si]));
                    }
                }
                else
                {
                    // Only one flag or option under one '-'
                    if (mDeclaredOptionsShortsMap.contains(arg[1]))
                    {
                        if (i >= cmdArgs.size() - 1 || cmdArgs.at(i + 1).starts_with('-'))
                        {
                            std::cout << "Option: " << arg[1] << " requires a value\n";
                            return;
                        }

                        mOptions[mDeclaredOptionsShortsMap.at(arg[1])] = cmdArgs.at(i + 1);
                        ++i;
                    }
                    else if (mDeclaredFlagsShortsMap.contains(arg[1]))
                    {
                        mFlags.insert(mDeclaredFlagsShortsMap.at(arg[1]));
                    }
                }
            }
            else
            {
                std::cout << "Unknown argument: " << arg << "\n";
                return;
            }
        }
    }

    void printDeclared()
    {
        std::cout << "Options:\n";
        for (const auto &option : mDeclaredOptions)
        {
            std::cout << "  -" << option.shortName << ", --" << option.longName;
            if (!option.description.empty())
            {
                std::cout << " : " << option.description;
            }
        }

        std::cout << "\nFlags:\n";
        for (const auto &flag : mDeclaredFlags)
        {
            std::cout << "  -" << flag.shortName << ", --" << flag.longName << "\n";
        }
    }

    void printHelp()
    {
        std::cout << mProgramDescription << "\n\n";
        printDeclared();
    }

private:
    struct Parameter
    {
        char shortName;
        std::string longName;
        bool required;
        std::string description;

        Parameter(char shortName, const std::string &longName, bool required = false, const std::string &description = "")
            : shortName(shortName), longName(longName), required(required), description(description) {}
        Parameter(const Parameter &other) = default;

        bool operator==(const Parameter &other) const
        {
            return shortName == other.shortName && longName == other.longName;
        }

        bool operator==(const char c) const
        {
            return shortName == c;
        }

        bool operator==(const std::string &s) const
        {
            return longName == s;
        }

        struct Hash
        {
            std::size_t operator()(const Parameter &param) const
            {
                return std::hash<char>()(param.shortName) ^ std::hash<std::string>()(param.longName);
            }
        };
    };

    int mArgc;
    char **mArgv;
    std::string mProgramDescription;
    std::unordered_map<Parameter, std::string, Parameter::Hash> mOptions;
    std::unordered_set<Parameter, Parameter::Hash> mFlags;

    std::unordered_set<Parameter, Parameter::Hash> mDeclaredOptions;
    std::unordered_set<Parameter, Parameter::Hash> mDeclaredFlags;

    std::unordered_map<char, Parameter> mDeclaredOptionsShortsMap;
    std::unordered_map<std::string, Parameter> mDeclaredOptionsLongsMap;
    std::unordered_map<char, Parameter> mDeclaredFlagsShortsMap;
    std::unordered_map<std::string, Parameter> mDeclaredFlagsLongsMap;
};
