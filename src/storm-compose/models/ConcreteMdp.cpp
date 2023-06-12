#include "ConcreteMdp.h"

namespace storm {
namespace models {

template<typename ValueType>
bool ConcreteMdp<ValueType>::isConcreteMdp() {
    return true;
}

}
}
