#include "SequenceModel.h"

namespace storm {
namespace models {

template <typename ValueType>
SequenceModel<ValueType>::SequenceModel(OpenMdpManager<ValueType>& manager, std::vector<std::shared_ptr<OpenMdp<ValueType>>> values) : OpenMdp<ValueType>(manager), values(values) {
}

template class SequenceModel<storm::RationalNumber>;
template class SequenceModel<double>;

}
}
