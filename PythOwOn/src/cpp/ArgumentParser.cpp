#include <stdexcept>
#include <sstream>

#include "fmt/core.h"

#include "Common.hpp"
#include "ArgumentParser.hpp"


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
        auto defaultArgIt = std::make_pair(
            longArgs.find(defaultArg),
            aliasArgs.find(defaultArg[0])
        );

        // execute the callback from the first non-end iterator
        if (defaultArgIt.first != longArgs.end()) {
            defaultArgIt.first->second.callback("");
        } else if (defaultArgIt.second != aliasArgs.end()) {
            defaultArgIt.second->second.callback("");
        } else {
            FMT_PRINT("This shouldn't happen...");
            exit(-1);
        }
    }
}

void ArgumentParser::defaultError() {
    if (errorArg.empty()) {
        throw std::invalid_argument("No default error set");
    } else {
        auto errorArgIt = std::make_pair(
            longArgs.find(errorArg),
            aliasArgs.find(errorArg[0])
        );
        
        // execute the callback from the first non-end iterator
        if (errorArgIt.first != longArgs.end()) {
            errorArgIt.first->second.callback("");
        } else if (errorArgIt.second != aliasArgs.end()) {
            errorArgIt.second->second.callback("");
        } else {
            FMT_PRINT("This shouldn't happen...");
            exit(-1);
        }
    }
}

void ArgumentParser::setDefaultBehaviour(std::string defaultArg) {
    if (longArgs.contains(defaultArg) || (aliasArgs.contains(defaultArg[0]) && defaultArg.length() == 1)) {
        this->defaultArg = defaultArg;
    } else {
        throw std::invalid_argument("Unknown/Unset argument when setting defaultBehaviour: " + defaultArg);
    }
}

void ArgumentParser::setDefaultError(std::string errorArg) {
    if (longArgs.contains(errorArg) || (aliasArgs.contains(errorArg[0]) && errorArg.length() == 1)){
        this->errorArg = errorArg;
    } else {
        throw std::invalid_argument("Unknown/Unset argument when setting defaultError: " + errorArg);
    }
}

void ArgumentParser::setDefaultHelp() {
    auto printHelp = [&](std::string _) {
        FMT_PRINT("Usage: {} [options]\n\nOptions:\n", args[0]);
        // print out help information for each long argument
        size_t maxLongArgLength = 0;
        for (auto const& [key, value] : longArgs) {
            Arg arg = value;
            std::string longArg = "--" + key;
            maxLongArgLength = std::max(maxLongArgLength, longArg.length());
        }

        for (auto const& [key, value] : longArgs) {
            Arg arg = value;
            std::string longArg = "--" + key;
            std::string shortArg = "";
            if (arg.alias != '\0') {
                shortArg = "-" + std::string(1, arg.alias);
            }

            std::string reqVal = fmt::format("{:<7}", arg.requiresArg ? "<VALUE>" : "");
            // TODO: make this dynamic from the length of the longest argument
            FMT_PRINT("{:<5} {} {:<{}} {}\n", shortArg, longArg, reqVal, 21 + reqVal.length() - longArg.length(), arg.help);
        }
    };

    registerLongArg("help", {"Print this help message", printHelp});
    aliasLongArg("help", 'h');

    setDefaultBehaviour("help");
    setDefaultError("help");
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
    std::vector<std::pair<Arg*, std::string>> toExecute;

    if (args.size() == 1) {
        defaultBehaviour();
        exit(0);
    }

    for (size_t i = 1; i < args.size(); i++) {
        std::string arg = args[i];

        if (arg[0] != '-') {
            continue;
        }

        Arg* argInfo = nullptr;

        if (arg[1] == '-') {
            std::string longArg = arg.substr(2);
            if (longArgs.find(longArg) != longArgs.end()) {
                argInfo = &longArgs[longArg];
            }
        } else {
            char shortArg = arg[1];
            if (arg.length() == 2 && (aliasArgs.find(shortArg) != aliasArgs.end())) {
                argInfo = &aliasArgs[shortArg];
            }
        }

        if (!argInfo) {
            FMT_PRINT("Unknown argument '{}'!\n", arg);
            defaultError();
            exit(0);
        } else if (argInfo->requiresArg && (i + 1 >= args.size() || args[i + 1][0] == '-')) {
            FMT_PRINT("Error: Argument '{}' requires a value!\n", arg);
            defaultError();
            exit(0);
        } else {
            toExecute.push_back({
                argInfo,
                argInfo->requiresArg ? args[++i] : ""
            });
        }
    }

    for (auto& arg : toExecute) {
        arg.first->callback(arg.second);
    }


}
