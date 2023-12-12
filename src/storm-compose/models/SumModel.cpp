#include "SumModel.h"

namespace storm {
namespace models {

template<typename ValueType>
SumModel<ValueType>::SumModel(std::shared_ptr<OpenMdpManager<ValueType>> manager, std::vector<std::shared_ptr<OpenMdp<ValueType>>> values)
    : OpenMdp<ValueType>(manager), values(values) {}

template<typename ValueType>
void SumModel<ValueType>::accept(visitor::OpenMdpVisitor<ValueType>& visitor) {
    visitor.visitSumModel(*this);
}

template<typename ValueType>
std::vector<std::shared_ptr<OpenMdp<ValueType>>>& SumModel<ValueType>::getValues() {
    return values;
}

template<typename ValueType>
bool SumModel<ValueType>::isRightward() const {
    for (const auto& value : values) {
        if (!value->isRightward()) {
            return false;
        }
    }
    return true;
}

template class SumModel<storm::RationalNumber>;
template class SumModel<double>;

}  // namespace models
}  // namespace storm
