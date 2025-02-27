#include "DfaModelChecking.h"
#include <mata/nfa/nfa.hh>

#include "storm/exceptions/UnexpectedException.h"
#include "storm/settings/SettingsManager.h"
#include "storm-compose-cli/settings/modules/ComposeIOSettings.h"
#include "storm/settings/modules/IOSettings.h"
#include "storm-compose/parser/DfaParser.h"
#include "storm-compose/dfa/DfaMdpProductPrism.h"
#include "storm-compose/dfa/DfaMdpProductChecker.h"
#include "storm-parsers/parser/PrismParser.h"

namespace storm {
namespace compose {
namespace cli {

template<typename ValueType>
void performDfaModelChecking() {
    auto& composeSettings = storm::settings::getModule<storm::settings::modules::ComposeIOSettings>();
    auto& ioSettings = storm::settings::getModule<storm::settings::modules::IOSettings>();

    std::ifstream inputFile(composeSettings.getDfaPropFilename());
    parser::DfaParser parser;
    parser.parseStream(inputFile);
    mata::nfa::Nfa dfa = parser.produceDfa();

    std::cout << "DFA:" << std::endl;
    dfa.print_to_dot(std::cout);
    auto minimized = mata::nfa::minimize(dfa);
    std::cout << "Miniminized:" << std::endl;
    minimized.print_to_dot(std::cout);

    if (!ioSettings.isPrismInputSet()) return;

    std::string const path = ioSettings.getPrismInputFilename();
    auto program = storm::parser::PrismParser::parse(path, false);
    auto mapping = parser.getMapping();

    if (!checkCompatibility<ValueType>(dfa,  mapping, program)) {
        throw exceptions::UnexpectedException("Labels of DFA is not a subset of labels of MDP: may not be compatible");
    }

    dfa::DfaMdpProductChecker<ValueType> checker({dfa, program, mapping});
    ValueType value = checker.check();

    std::cout << "Value: " << value << std::endl;
}

template<typename ValueType>
bool checkCompatibility(mata::nfa::Nfa const& dfa, parser::DfaMapping const& mapping, storm::prism::Program const& program) {
    const auto& programLabels = program.getLabels();
    const auto& dfaLabels = mapping.inputMapping;

    bool disjoint = true;
    bool subset = true;

    for (const auto& [key, value]: dfaLabels) {
        if (program.hasLabel(key)) {
            disjoint = false;

            std::cout << "MDP and DFA synchronize on label '" << key << "'" << std::endl;
            //break;
        } else {
            std::cout << key << " present in DFA but not in MDP" << std::endl;
            subset = false;
        }
    }

    return subset;
}

template void performDfaModelChecking<double>();
template void performDfaModelChecking<storm::RationalNumber>();

template bool checkCompatibility<double>(mata::nfa::Nfa const&, parser::DfaMapping const&, storm::prism::Program const&);
template bool checkCompatibility<storm::RationalNumber>(mata::nfa::Nfa const&, parser::DfaMapping const&, storm::prism::Program const&);  // namespace cli

}  // namespace cli
}  // namespace compose
}  // namespace storm
