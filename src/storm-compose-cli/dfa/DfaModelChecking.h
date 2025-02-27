#ifndef DFAMODELCHECKING_H_
#define DFAMODELCHECKING_H_

#include <mata/nfa/nfa.hh>
#include "storm/storage/prism/Program.h"
#include "storm-compose/parser/DfaParser.h"

namespace storm {
namespace compose {
namespace cli {

template<typename ValueType>
void performDfaModelChecking();

template<typename ValueType>
bool checkCompatibility(mata::nfa::Nfa const& dfa, parser::DfaMapping const& mapping, storm::prism::Program const& program);

}  // namespace cli
}  // namespace compose
}  // namespace storm

#endif  // DFAMODELCHECKING_H_
