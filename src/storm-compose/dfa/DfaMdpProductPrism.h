#ifndef DFAMDPPRODUCTPRISM_H
#define DFAMDPPRODUCTPRISM_H

#include <mata/nfa/nfa.hh>

#include "storm/storage/prism/Program.h"
#include "storm-compose/parser/DfaParser.h"

namespace storm {
namespace compose {
namespace dfa {

class DfaMdpProductPrism {
   public:
    DfaMdpProductPrism(storm::parser::DfaMapping const& mapping);
    void writeProductAsPrism(mata::nfa::Nfa& nfa, storm::prism::Program const& program, std::ostream& out);

   private:
    void writeNfaAsPrismModule(mata::nfa::Nfa& nfa, storm::prism::Program const& program, std::ostream& out);

    storm::parser::DfaMapping const& mapping;
};

}  // namespace dfa
}  // namespace compose
}  // namespace storm

#endif /* DFAMDPPRODUCTPRISM_H */
