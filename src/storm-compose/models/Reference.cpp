#include "Reference.h"

#include "storm/adapters/RationalNumberAdapter.h"

namespace storm {
namespace models {

template<typename ValueType>
Reference<ValueType>::Reference(OpenMdpManager<ValueType>& manager, std::string reference) : OpenMdp<ValueType>(manager), reference(reference) {

}

template<typename ValueType>
bool Reference<ValueType>::isReference() {
    return true;
}

template<typename ValueType>
std::string Reference<ValueType>::getReference() {
    return reference;
}

template class Reference<storm::RationalNumber>;
template class Reference<double>;

}
}
