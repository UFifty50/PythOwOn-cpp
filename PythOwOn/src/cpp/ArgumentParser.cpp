#include "ArgumentParser.hpp"

#include <algorithm>
#include <ranges>
#include <set>
#include <sstream>
#include <stdexcept>

#include "fmt/core.h"

#include "Common.hpp"


ArgumentParser::ArgumentParser(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        this->argv.push_back(argv[i]);
    }
}

ArgumentParser::~ArgumentParser() { argv.clear(); }


uint8_t ArgumentParser::defaultBehaviour() {
    if (defaultArg.empty()) {
        FMT_PRINT("Error: {0}\n",
                  "No default behaviour set when calling defaultBehaviour");
        return 1;
    } else {
        return defaultT(defaultArg);
    }
}

uint8_t ArgumentParser::defaultError() {
    if (errorArg.empty()) {
        FMT_PRINT("Error: No default error set when calling defaultError\n");
        return 1;
    } else {
        return defaultT(defaultArg);
    }
}

uint8_t ArgumentParser::defaultT(std::string arg) {
    auto argIterator = std::make_pair(longArgs.find(arg), aliasArgs.find(arg[0]));

    // execute the callback from the first non-end iterator
    if (argIterator.first != longArgs.end()) {
        return argIterator.first->second.callback("");
    } else if (argIterator.second != aliasArgs.end()) {
        return argIterator.second->second.callback("");
    } else {
        FMT_PRINT("This shouldn't happen...");
        return 1;
    }
}

void ArgumentParser::setDefaultBehaviour(std::string defaultArg) {
    if (longArgs.contains(defaultArg) ||
        (aliasArgs.contains(defaultArg[0]) && defaultArg.length() == 1)) {
        this->defaultArg = defaultArg;
    } else {
        FMT_PRINT("Error: Unknown/Unset argument when setting defaultBehaviour: {}\n",
                  defaultArg);
        exit(1);
    }
}

void ArgumentParser::setDefaultError(std::string errorArg) {
    if (longArgs.contains(errorArg) ||
        (aliasArgs.contains(errorArg[0]) && errorArg.length() == 1)) {
        this->errorArg = errorArg;
    } else {
        FMT_PRINT("Error: Unknown/Unset argument when setting defaultError: {}\n",
                  errorArg);
        exit(1);
    }
}

void ArgumentParser::setDefaultHelp() {
    auto printHelp = [&](std::string _) -> uint8_t {
        FMT_PRINT("Usage: {} [options]\n\nOptions:\n", argv[0]);
        // print out help information for each long argument
        size_t maxLongArgLength = 0;
        std::string helpString;

        for (const auto& [key, arg] : longArgs) {
            std::string longArg = "--" + key;
            std::string shortArg =
                arg.alias != '\0' ? "-" + std::string(1, arg.alias) : "";

            maxLongArgLength = std::max(maxLongArgLength, longArg.length());

            std::string reqVal = FMT_FORMAT("{:<7}", arg.requiresArg ? "<VALUE>" : "");
            // TODO: make this dynamic from the length of the longest argument
            helpString += FMT_FORMAT("{:<5} {} {:<{}} {}\n", shortArg, longArg, reqVal,
                                     21 + reqVal.length() - longArg.length(), arg.help);
        }

        FMT_PRINT(helpString);
        return 0;
    };

    registerLongArg("help", {"Print this help message", printHelp});
    aliasLongArg("help", 'h');

    setDefaultBehaviour("help");
    setDefaultError("help");
}

void ArgumentParser::registerLongArg(std::string argName, Arg arg) {
    longArgs[argName] = arg;
}

void ArgumentParser::aliasLongArg(std::string arg, char alias) {
    auto it = longArgs.find(arg);
    if (it == longArgs.end()) {
        FMT_PRINT("Error: Argument {0} did not exist when aliasing\n", arg);
        exit(1);
    }
    aliasArgs[alias] = it->second;
    it->second.alias = alias;
}

uint8_t ArgumentParser::parse() {
    std::vector<std::pair<Arg*, std::string>> toExecute;
    std::set<uint8_t> returnVals;

    if (argv.size() == 1) {
        return defaultBehaviour();
    }

    for (auto it = argv.begin(); it != argv.end(); it++) {
        std::string arg = *it;

        if (arg[0] != '-') continue;

        Arg* argInfo = nullptr;

        if (arg[1] == '-') {
            std::string longArg = arg.substr(2);
            if (longArgs.find(longArg) != longArgs.end()) {
                argInfo = &longArgs[longArg];
            }
        } else {
            char shortArg = arg[1];
            if (isArg(shortArg, aliasArgs) && arg.length() == 2) {
                argInfo = &aliasArgs[shortArg];
            }
        }

        if (!argInfo) {
            FMT_PRINT("Unknown argument '{}'!\n", arg);
            defaultError();
            return 1;

        } else if (argInfo->requiresArg && (arg == argv.back() || (it[1][0] == '-'))) {
            FMT_PRINT("Error: Argument '{}' requires a value!\n", arg);
            defaultError();
            return 1;

        } else {
            toExecute.push_back({argInfo, argInfo->requiresArg ? *++it : ""});
        }
    }

    for (auto& arg : toExecute) {
        returnVals.insert(arg.first->callback(arg.second));
    }

    // if any of returnVals != 0
    if (std::ranges::any_of(returnVals, [](uint8_t val) { return val != 0; })) {
        return 1;
    }
    return 0;
}
