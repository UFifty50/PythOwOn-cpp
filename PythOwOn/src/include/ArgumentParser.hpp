#ifndef ARGPARSE_HPP
#define ARGPARSER_HPP

#include <functional>
#include <map>
#include <string>
#include <vector>


struct Arg {
    std::string help;
    std::function<uint8_t(std::string)> callback;
    bool requiresArg = false;
    char alias = '\0';
};

class ArgumentParser {
private:
    std::vector<std::string> argv;
    std::unordered_map<char, Arg> aliasArgs;
    std::unordered_map<std::string, Arg> longArgs;
    std::string defaultArg;
    std::string errorArg;

    uint8_t defaultBehaviour();
    uint8_t defaultError();
    uint8_t defaultT(std::string arg);

    template <typename T>
    bool isArg(T arg, std::unordered_map<T, Arg> argMap) {
        return (argMap.find(arg) != argMap.end());
    }

public:
    ArgumentParser(int argc, char** argv);
    ~ArgumentParser();

    void setDefaultBehaviour(std::string defaultArg);
    void setDefaultError(std::string defaultArg);
    void setDefaultHelp();

    void registerLongArg(std::string argName, Arg arg);
    void aliasLongArg(std::string arg, char alias);

    uint8_t parse();
};

#endif
