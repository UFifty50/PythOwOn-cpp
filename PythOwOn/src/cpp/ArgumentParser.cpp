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

template<typename T>
inline bool ArgumentParser::isArg(T arg, std::map<T, Arg> argMap) {
    return (argMap.find(arg) != argMap.end())
}


void ArgumentParser::defaultBehaviour() {
    if (defaultArg.empty()) {
        try {
            throw std::invalid_argument("No default behaviour set when calling defaultBehaviour");
        } catch (std::invalid_argument& e) {
            FMT_PRINT("Error: {}\n", e.what());
            exit(-1);
        }
    } else {
        defaultT(defaultArg);
    }
}

void ArgumentParser::defaultError() {
    if (errorArg.empty()) {
        try {
            throw std::invalid_argument("No default error set when calling defaultError");
        } catch (std::invalid_argument& e) {
            FMT_PRINT("Error: {}\n", e.what());
            exit(-1);
        }
    } else {
        defaultT(defaultArg);
    }
}

void ArgumentParser::defaultT(std::string arg) {
    auto argIterator = std::make_pair(
        longArgs.find(arg),
        aliasArgs.find(arg[0])
    );

    // execute the callback from the first non-end iterator
    if (argIterator.first != longArgs.end()) {
        argIterator.first->second.callback("");
    } else if (argIterator.second != aliasArgs.end()) {
        argIterator.second->second.callback("");
    } else {
        FMT_PRINT("This shouldn't happen...");
        exit(-1);
    }
}

void ArgumentParser::setDefaultBehaviour(std::string defaultArg) {
    if (longArgs.contains(defaultArg) || (aliasArgs.contains(defaultArg[0]) && defaultArg.length() == 1)) {
        this->defaultArg = defaultArg;
    } else {
        try {
            throw std::invalid_argument("Unknown/Unset argument when setting defaultBehaviour: " + defaultArg);
        } catch (std::invalid_argument& e) {
            FMT_PRINT("Error: {}\n", e.what());
            exit(-1);
        }
    }
}

void ArgumentParser::setDefaultError(std::string errorArg) {
    if (longArgs.contains(errorArg) || (aliasArgs.contains(errorArg[0]) && errorArg.length() == 1)){
        this->errorArg = errorArg;
    } else {
        try {
            throw std::invalid_argument("Unknown/Unset argument when setting defaultError: " + errorArg);
        } catch (std::invalid_argument& e) {
            FMT_PRINT("Error: {}\n", e.what());
            exit(-1);
        }
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

void ArgumentParser::registerLongArg(std::string argName, Arg arg) {
    longArgs[argName] = arg;
}

void ArgumentParser::aliasLongArg(std::string arg, char alias) {
    auto it = longArgs.find(arg);
    if (it == longArgs.end()) {
        try {
            throw std::invalid_argument("Argument " + arg + " does not exist during aliasing");
        } catch (std::invalid_argument& e) {
            FMT_PRINT("Error: {}\n", e.what());
            exit(-1);
        }
    }
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
            if (isArg(shortArg, aliasArgs) && arg.length() == 2) {
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
