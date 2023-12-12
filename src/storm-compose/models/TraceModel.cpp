#include "TraceModel.h"

namespace storm {
namespace models {

template<typename ValueType>
TraceModel<ValueType>::TraceModel(std::shared_ptr<OpenMdpManager<ValueType>> manager, std::shared_ptr<OpenMdp<ValueType>> value, size_t left, size_t right)
    : OpenMdp<ValueType>(manager), value(value), left(left), right(right) {}

template<typename ValueType>
void TraceModel<ValueType>::accept(visitor::OpenMdpVisitor<ValueType>& visitor) {
    visitor.visitTraceModel(*this);
}

template<typename ValueType>
bool TraceModel<ValueType>::isRightward() const {
    if (left > 0) {
        return false;
    }

    return value->isRightward();
}

template<typename ValueType>
std::shared_ptr<OpenMdp<ValueType>> TraceModel<ValueType>::getValue() {
    return value;
}

template<typename ValueType>
size_t TraceModel<ValueType>::getLeft() {
    return left;
}

template<typename ValueType>
size_t TraceModel<ValueType>::getRight() {
    return right;
}

template class TraceModel<storm::RationalNumber>;
template class TraceModel<double>;

}  // namespace models
}  // namespace storm
