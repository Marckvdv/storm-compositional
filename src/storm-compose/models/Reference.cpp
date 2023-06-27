#include "Reference.h"

#include "storm/adapters/RationalNumberAdapter.h"

namespace storm {
namespace models {

template<typename ValueType>
Reference<ValueType>::Reference(OpenMdpManager<ValueType>& manager, std::string reference) : OpenMdp<ValueType>(manager), reference(reference) {

}

template<typename ValueType>
bool Reference<ValueType>::isReference() const {
    return true;
}

template<typename ValueType>
std::string Reference<ValueType>::getReference() const {
    return reference;
}

template <typename ValueType>
void Reference<ValueType>::accept(visitor::OpenMdpVisitor<ValueType>& visitor) {
    visitor.visitReference(*this);
}

template <typename ValueType>
std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> Reference<ValueType>::collectEntranceExit(typename OpenMdp<ValueType>::EntranceExit entranceExit, typename OpenMdp<ValueType>::Scope& scope) const {
    auto openMdp = this->manager.dereference(reference);
    return openMdp->collectEntranceExit(entranceExit, scope);
}

template class Reference<storm::RationalNumber>;
template class Reference<double>;

}
}
