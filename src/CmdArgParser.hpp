#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <iostream>
#include <vector>

namespace parser
{
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

std::ostream &operator<<(std::ostream &stream, const Parameter &param)
{
    stream << "  -" << param.shortName << ", --" << param.longName;
    if (!param.description.empty())
    {
        std::cout << " : " << param.description;
    }
    return stream;
}

class CmdArgParser
{
public:
    // Constructors
    CmdArgParser(int argc, char **argv, std::string programDescription = "")
        : mArgc(argc), mArgv(argv), mProgramDescription(std::move(programDescription))
    {
        declareFlag('h', "help", false, "Display this help message");
    }

    // Parsers
    bool parseArgs()
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
                return true;
            }
            else if (arg.starts_with("--") && arg.size() > 2)
            {
                const auto longName = arg.substr(2);

                if (mDeclaredOptionsLongsMap.contains(longName))
                {
                    if (i >= cmdArgs.size() - 1 || cmdArgs.at(i + 1).starts_with('-'))
                    {
                        reportError("Option: ", longName, " requires a value");
                        return false;
                    }

                    const auto &param = mDeclaredFlagsLongsMap.at(longName);
                    if (mOptions.contains(param))
                    {
                        reportError("Option: ", param, " already used");
                        return false;
                    }

                    mOptions[mDeclaredOptionsLongsMap.at(longName)] = cmdArgs.at(i + 1);
                    ++i;
                }
                else if (mDeclaredFlagsLongsMap.contains(longName))
                {
                    const auto &param = mDeclaredFlagsLongsMap.at(longName);
                    if (mFlags.contains(param))
                    {
                        reportError("Flag: ", param, " already used");
                        return false;
                    }

                    mFlags.insert(param);
                }
                else
                {
                    reportError("Undeclared parameter: ", longName);
                    return false;
                }
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
                            reportError("Cannot bundle options. Each option needs a separate usage. Option: ", arg[si], " bundled in phrase: ", arg);
                            return false;
                        }

                        if (!mDeclaredFlagsShortsMap.contains(arg[si]))
                        {
                            reportError("Undeclared flag: ", arg[si]);
                            return false;
                        }

                        const auto& param = mDeclaredFlagsShortsMap.at(arg[si]);
                        if (mFlags.contains(param))
                        {
                            reportError("Flag: ", param, " already used");
                            return false;
                        }

                        mFlags.insert(param);
                    }
                }
                else
                {
                    // Only one flag or option under one '-'
                    if (mDeclaredOptionsShortsMap.contains(arg[1]))
                    {
                        if (i >= cmdArgs.size() - 1 || cmdArgs.at(i + 1).starts_with('-'))
                        {
                            reportError("Option: ", arg[1], " requires a value");
                            return false;
                        }

                        const auto &param = mDeclaredOptionsShortsMap.at(arg[1]);
                        if (mOptions.contains(param))
                        {
                            reportError("Option: ", param, " already used");
                            return false;
                        }

                        mOptions[mDeclaredOptionsShortsMap.at(arg[1])] = cmdArgs.at(i + 1);
                        ++i;
                    }
                    else if (mDeclaredFlagsShortsMap.contains(arg[1]))
                    {
                        const auto &param = mDeclaredFlagsShortsMap.at(arg[1]);
                        if (mFlags.contains(param))
                        {
                            reportError("Flag: ", param, " already used");
                            return false;
                        }

                        mFlags.insert(param);
                    }
                    else
                    {
                        reportError("Undeclared parameter: ", arg[1]);
                        return false;
                    }
                }
            }
            else
            {
                reportError("Unknown argument: ", arg);
                return false;
            }
        }

        return true;
    }

    // Declarators
    bool declareOption(const char shortName, const std::string &longName, bool required = false, std::string description = "")
    {
        if (checkDeclarationPossibility(shortName, longName))
        {
            Parameter param{shortName, longName, required, description};
            mDeclaredOptionsShortsMap.emplace(shortName, param);
            mDeclaredOptionsLongsMap.emplace(longName, param);
            mDeclaredOptions.insert(param);
            return true;
        }

        return false;
    }

    bool declareFlag(const char shortName, const std::string &longName, bool required = false, std::string description = "")
    {
        if (checkDeclarationPossibility(shortName, longName))
        {
            Parameter param{shortName, longName, required, description};
            mDeclaredFlagsShortsMap.emplace(shortName, param);
            mDeclaredFlagsLongsMap.emplace(longName, param);
            mDeclaredFlags.insert(param);
            return true;
        }

        return false;
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

    bool hasFlag(char shortName, const std::string& longName) const
    {
        return mFlags.contains({shortName, longName});
    }

private:
    bool checkDeclarationPossibility(const char shortName, const std::string &longName)
    {
        if (mDeclaredFlagsShortsMap.contains(shortName) || mDeclaredFlagsLongsMap.contains(longName))
        {
            reportError("Unable to declare: -", shortName, " , --", longName, ". Already declared in flags");
            printDeclared();
            return false;
        }

        if (mDeclaredOptionsShortsMap.contains(shortName) || mDeclaredOptionsLongsMap.contains(longName))
        {
            reportError("Unable to declare: -", shortName, " , --", longName, ". Already declared in options");
            printDeclared();
            return false;
        }

        return true;
    }

    void printDeclared()
    {
        std::cout << "Declared parameters:\n";
        std::cout << "Options:\n";
        for (const auto &option : mDeclaredOptions)
        {
            std::cout << option << "\n";
        }

        std::cout << "\nFlags:\n";
        for (const auto &flag : mDeclaredFlags)
        {
            std::cout << flag << "\n";
        }
    }

    void printHelp()
    {
        std::cout << mProgramDescription << "\n\n";
        printDeclared();
    }

    void reportError()
    {
        std::cout << std::endl;
    }

    template <typename T, typename... Args>
    void reportError(T firstArg, Args... remainingArgs)
    {
        std::cout << firstArg;
        reportError(remainingArgs...);
    }

private:
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
}
