#include "DfaParser.h"
#include <cstddef>
#include <mata/nfa/nfa.hh>
#include <mata/utils/sparse-set.hh>
#include <sstream>

#include "storm/exceptions/UnexpectedException.h"
#include "storm/utility/macros.h"
#include <unordered_map>

namespace storm {
namespace parser {

const std::string TERMINAL_SPECIFIER = "terminals:";
const std::string INITIAL_SPECIFIER = "initial:";

DfaParser::DfaParser() {}

void DfaParser::parseStream(std::istream& input) {
    while (input.good()) {
        std::string line;
        std::getline(input, line);

        if (isTransition(line)) {
            auto newTransition = parseTransition(line);
            transitions.push_back(newTransition);
        } else if (isTerminal(line)) {
            auto newTerminals = parseTerminals(line);
            terminals.insert(terminals.end(), newTerminals.begin(), newTerminals.end());
        } else if (isInitial(line)) {
            auto newInitial = parseInitial(line);
            initial = newInitial;
        } else if (line.length() != 0) {
            throw exceptions::UnexpectedException("failed to parse dfa");
        }
    }
}

mata::nfa::Nfa DfaParser::produceDfa() {
    auto addToMap = [](auto& mapping, auto& reverseMapping, StateType const& id) {
        auto it = mapping.find(id);
        if (it == mapping.end()) {
            size_t index = mapping.size();
            mapping[id] = index;
            reverseMapping[index] = id;
        }
    };

    for (const auto& transition : transitions) {
        addToMap(mapping.stateMapping, mapping.reverseStateMapping, transition.source);
        addToMap(mapping.inputMapping, mapping.reverseInputMapping, transition.input);
        addToMap(mapping.stateMapping, mapping.reverseStateMapping, transition.destination);
    }
    for (const auto& terminal : terminals) {
        addToMap(mapping.stateMapping, mapping.reverseStateMapping, terminal);
    }

    mata::nfa::Nfa nfa(mapping.stateMapping.size());
    for (const auto& transition : transitions) {
        nfa.delta.add(mapping.stateMapping[transition.source], mapping.inputMapping[transition.input], mapping.stateMapping[transition.destination]);
    }

    std::vector<size_t> finalStates;
    for (const auto& terminal : terminals) {
        nfa.final.insert(mapping.stateMapping[terminal]);
    }
    if (!initial) {
        throw exceptions::UnexpectedException("no initial state");
    }
    nfa.initial = {mapping.stateMapping[*initial]};

    return nfa;
}

DfaMapping DfaParser::getMapping() {
    return mapping;
}

std::vector<DfaParser::StateType> DfaParser::parseTerminals(std::string const& s) {
    STORM_LOG_ASSERT(isTerminal(s), "not a terminal");

    std::vector<StateType> newTerminals;
    std::istringstream remaining(s.substr(TERMINAL_SPECIFIER.length()));

    std::string word;
    while (remaining >> word) {
        newTerminals.push_back(word);
    }

    return newTerminals;
}

DfaTransition DfaParser::parseTransition(std::string const& s) {
    std::istringstream remaining(s);

    std::string source, arrow, dest;
    remaining >> source;
    remaining >> arrow;
    remaining >> dest;

    std::string input;
    if (arrow.length() >= 3 && *arrow.begin() == '-' && *(arrow.end()-1) == '>') {
        input = arrow.substr(1, arrow.length()-2);
    } else {
        throw exceptions::UnexpectedException("incorrect transition");
    }
    std::cout << "Parsed transition from " << source << " to " << dest << " with input " << input << std::endl;

    return {source, input, dest};
}

DfaParser::StateType DfaParser::parseInitial(std::string const& s) {
    STORM_LOG_ASSERT(isInitial(s), "not initial");
    if (initial) {
        throw exceptions::UnexpectedException("initial state already specified!");
    }

    std::istringstream remaining(s.substr(INITIAL_SPECIFIER.length()));
    std::string newInitial;
    remaining >> newInitial;

    return newInitial;
}

bool DfaParser::isTransition(std::string const& s) {
    return !isTerminal(s) && !isInitial(s) && s.length() > 0;
}

bool DfaParser::isTerminal(std::string const& s) {
    return s.rfind(TERMINAL_SPECIFIER, 0) == 0;
}

bool DfaParser::isInitial(std::string const& s) {
    return s.rfind(INITIAL_SPECIFIER, 0) == 0;
}

}  // namespace parser
}  // namespace storm
