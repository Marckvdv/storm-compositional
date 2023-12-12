#include "SequenceModel.h"

namespace storm {
namespace models {

template<typename ValueType>
SequenceModel<ValueType>::SequenceModel(std::shared_ptr<OpenMdpManager<ValueType>> manager, std::vector<std::shared_ptr<OpenMdp<ValueType>>> values)
    : OpenMdp<ValueType>(manager), values(values) {}

template<typename ValueType>
void SequenceModel<ValueType>::accept(visitor::OpenMdpVisitor<ValueType>& visitor) {
    visitor.visitSequenceModel(*this);
}

template<typename ValueType>
std::vector<std::shared_ptr<OpenMdp<ValueType>>>& SequenceModel<ValueType>::getValues() {
    return values;
}

template<typename ValueType>
bool SequenceModel<ValueType>::isRightward() const {
    for (const auto& value : values) {
        if (!value->isRightward()) {
            return false;
        }
    }
    return true;
}

template class SequenceModel<storm::RationalNumber>;
template class SequenceModel<double>;

}  // namespace models
}  // namespace storm
