#include "DfaMdpProductCVI.h"

namespace storm {
namespace compose {
namespace dfa {

template<typename ValueType>
DfaMdpProductCVI<ValueType>::DfaMdpProductCVI(DfaMdpProduct dfaProduct) : dfaProduct(dfaProduct) {

}

template<typename ValueType>
void DfaMdpProductCVI<ValueType>::constructValueVector() {
    // We need to construct a value vector which contains
    // |Q|*|O| values, where
    // - |Q| is the number of DFA states
    // - |O| is the number of exits for each canonical oMDP.
    //
}

}  // namespace dfa
}  // namespace compose
}  // namespace storm
