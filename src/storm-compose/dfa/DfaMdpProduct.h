#ifndef DFAMDPPRODUCT_H_
#define DFAMDPPRODUCT_H_

#include <mata/nfa/nfa.hh>
#include "storm/storage/prism/Program.h"
#include "storm-compose/parser/DfaParser.h"

namespace storm {
namespace compose {
namespace dfa {

struct DfaMdpProduct {
    mata::nfa::Nfa& dfa;
    storm::prism::Program const& program;
    storm::parser::DfaMapping const& mapping;
};

}  // namespace dfa
}  // namespace compose
}  // namespace storm

#endif // DFAMDPPRODUCT_H_
