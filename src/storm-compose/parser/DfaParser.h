#ifndef DFAPARSER_H
#define DFAPARSER_H

#include <mata/nfa/nfa.hh>
#include <iostream>
#include <optional>
#include <vector>

namespace storm {
namespace parser {

struct DfaTransition {
    std::string source, input, destination;
};

struct DfaMapping {
    std::unordered_map<std::string, std::size_t> stateMapping, inputMapping;
    std::unordered_map<std::size_t, std::string> reverseStateMapping, reverseInputMapping;
};

class DfaParser {
   public:
    typedef std::string StateType;

    DfaParser();

    void parseStream(std::istream& input);
    mata::nfa::Nfa produceDfa();
    DfaMapping getMapping();

   private:
    bool isTransition(std::string const& s);
    bool isTerminal(std::string const& s);
    bool isInitial(std::string const& s);
    std::vector<StateType> parseTerminals(std::string const& s);
    DfaTransition parseTransition(std::string const& s);
    StateType parseInitial(std::string const& s);

    std::vector<DfaTransition> transitions;
    std::vector<StateType> terminals;
    std::optional<std::string> initial;

    DfaMapping mapping;
};

}  // namespace parser
}  // namespace storm

#endif /* DFAPARSER_H */
