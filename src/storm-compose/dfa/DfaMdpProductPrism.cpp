#include "DfaMdpProductPrism.h"

namespace storm {
namespace compose {
namespace dfa {

#define STATE_IDENT "q__"
#define TARGET_IDENT "\"target__\""
#define MODULE_IDENT "dfa__"
#define NO_LABEL_IDENT "no_label__"

DfaMdpProductPrism::DfaMdpProductPrism(storm::parser::DfaMapping const& mapping) : mapping(mapping) {}

void DfaMdpProductPrism::writeProductAsPrism(mata::nfa::Nfa& nfa, storm::prism::Program const& program, std::ostream& out) {
    out << program << std::endl;

    writeNfaAsPrismModule(nfa, program, out);
}
void DfaMdpProductPrism::writeNfaAsPrismModule(mata::nfa::Nfa& dfa, storm::prism::Program const& program, std::ostream& out) {
    out << "formula " NO_LABEL_IDENT " = true";
    for (const auto& [label, idx] : mapping.inputMapping) {
        out << "&(!" << label << ")";
    }
    out << ";" << std::endl << std::endl;

    out << "module " MODULE_IDENT << std::endl;

    size_t stateCount = dfa.num_of_states();
    size_t initialState = *dfa.initial.begin();

    out << "\t" STATE_IDENT " : [0 .. " << stateCount-1 << "] init " << initialState << ";" << std::endl << std::endl;

    size_t source = 0;
    for (mata::nfa::StatePost const& transition : dfa.delta) {
        for (const auto& move : transition.moves()) {
            for (const auto& mdpAction : program.getActions()) {
                if (mdpAction == "") continue;

                std::string const& inputSymbol = mapping.reverseInputMapping.at(move.symbol);
                out <<
                    "\t[" << mdpAction << "] " <<
                    STATE_IDENT "=" << source << " & " << inputSymbol <<
                    " -> " << "(" STATE_IDENT "'=" << move.target << ");" << std::endl;
            }
            out << std::endl;
        }

        ++source;
    }

    for (const auto& mdpAction : program.getActions()) {
        if (mdpAction == "") continue;
        out << "\t[" << mdpAction << "] " NO_LABEL_IDENT " -> true;" << std::endl;
    }


    out << "endmodule" << std::endl << std::endl;
    out << "label " TARGET_IDENT " = false";
    for (const auto& state : dfa.final) {
        out << "|(" STATE_IDENT "=" << state << ")";
    }
    out << ";" << std::endl;

}

#undef MODULE_IDENT
#undef TARGET_IDENT
#undef STATE_IDENT
#undef NO_LABEL_IDENT

}  // namespace dfa
}  // namespace compose
}  // namespace storm
