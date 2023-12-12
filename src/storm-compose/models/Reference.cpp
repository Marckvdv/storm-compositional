#include "Reference.h"

#include "storm/adapters/RationalNumberAdapter.h"

namespace storm {
namespace models {

template<typename ValueType>
Reference<ValueType>::Reference(std::shared_ptr<OpenMdpManager<ValueType>> manager, std::string reference)
    : OpenMdp<ValueType>(manager), reference(reference) {}

template<typename ValueType>
bool Reference<ValueType>::isReference() const {
    return true;
}

template<typename ValueType>
std::string Reference<ValueType>::getReference() const {
    return reference;
}

template<typename ValueType>
void Reference<ValueType>::accept(visitor::OpenMdpVisitor<ValueType>& visitor) {
    visitor.visitReference(*this);
}

template<typename ValueType>
bool Reference<ValueType>::isRightward() const {
    auto openMdp = this->getManager()->dereference(reference);
    return openMdp->isRightward();
}

template class Reference<storm::RationalNumber>;
template class Reference<double>;

}  // namespace models
}  // namespace storm
