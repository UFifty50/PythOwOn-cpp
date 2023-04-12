#ifndef ARGPARSE_HPP
#define ARGPARSER_HPP

#include <vector>
#include <map>
#include <string>
#include <functional>


struct Arg {
    std::string help;
    std::function<void(std::string)> callback;
    bool requiresArg = false;
    char alias = '\0';
};

class ArgumentParser {
private:
    std::vector<std::string> args;
    std::map<char, Arg> aliasArgs;
    std::map<std::string, Arg> longArgs;
    std::string defaultArg;
    std::string errorArg;

    void defaultBehaviour();
    void defaultError();

public:
    ArgumentParser(int argc, char** argv);
    ~ArgumentParser();

    void setDefaultBehaviour(std::string defaultArg);
    void setDefaultError(std::string defaultArg);
    void setDefaultHelp();

    void registerLongArg(std::string argName, Arg arg);
    void aliasLongArg(std::string arg, char alias);

    void parse();
};

#endif
