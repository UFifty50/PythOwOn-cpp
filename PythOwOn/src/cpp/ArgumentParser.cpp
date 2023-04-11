#include <stdexcept>
#include <iostream>
#include <sstream>

#include "ArgumentParser.hpp"
#include <iomanip>


ArgumentParser::ArgumentParser(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        args.push_back(argv[i]);
    }
}

ArgumentParser::~ArgumentParser() {
    args.clear();
}

void ArgumentParser::defaultBehaviour() {
    if (defaultArg.empty()) {
        throw std::invalid_argument("No default behaviour set");
    } else {
        auto defaultArgIt = longArgs.find(defaultArg);
        if (defaultArgIt == longArgs.end())
            auto defaultArgIt = aliasArgs.find(defaultArg[0]);
        
        defaultArgIt->second.callback("");
    }
}

void ArgumentParser::setDefaultBehaviour(std::string defaultArg) {
    if (longArgs.count(defaultArg) || aliasArgs.count(defaultArg[0])) {
        this->defaultArg = defaultArg;
    } else {
        throw std::invalid_argument("Unknown argument when setting defaultBehaviour: " + defaultArg);
    }

}

void ArgumentParser::setDefaultHelp() {
    auto printHelp = [&](std::string _) {
        std::cout << "Usage: " << args[0] << " [options] [file]\n\n"
                  << "Options:\n";
        // print out help information for each long argument
        for (auto const& [key, value] : longArgs) {
            Arg arg = value;
            std::string longArg = "--" + key;
            std::string shortArg = "";
            if (arg.alias != '\0') {
                shortArg = "-" + std::string(1, arg.alias);
            }
            std::cout << std::setw(5) << std::left << shortArg << std::setw(17)
                      << longArg << arg.help << std::endl;
        }
    };

    registerLongArg("help", {"Print this help message", printHelp});
    aliasLongArg("help", 'h');

    setDefaultBehaviour("help");
}

void ArgumentParser::registerLongArg(std::string argName,
                                     Arg arg) {
    longArgs[argName] = arg;
}

void ArgumentParser::aliasLongArg(std::string arg, char alias) {
    auto it = longArgs.find(arg);
    if (it == longArgs.end())
        throw std::invalid_argument("Argument " + arg + " does not exist");
    aliasArgs[alias] = it->second;
    it->second.alias = alias;
}

void ArgumentParser::parse() {
    if (args.size() == 1) {
        defaultBehaviour();
        exit(0);
    }

    for (unsigned int i = 1; i < args.size(); i++) {
        std::string arg = args[i];
        if (arg[0] == '-' && arg[1] == '-') {
            if (longArgs.find(arg.substr(2)) != longArgs.end()) {
                if (i + 1 < args.size()) {
                    longArgs[arg.substr(2)].callback(args[i + 1]);
                    i++;
                } else {
                    longArgs[arg.substr(2)].callback("");
                }
            } else {
                defaultBehaviour();
                exit(0);
            }
        } else if (arg[0] == '-') {
            if (arg.length() == 2 &&
                aliasArgs.find(arg[1]) != aliasArgs.end()) {
                if (i + 1 < args.size()) {
                    aliasArgs[arg[1]].callback(args[i + 1]);
                    i++;
                } else {
                    aliasArgs[arg[1]].callback("");
                }
            } else {
                defaultBehaviour();
                exit(0);
            }
        } else {
            defaultBehaviour();
            exit(0);
        }
    }
}
