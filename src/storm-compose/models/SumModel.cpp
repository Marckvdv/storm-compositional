#include "SumModel.h"

namespace storm {
namespace models {

template <typename ValueType>
SumModel<ValueType>::SumModel(OpenMdpManager<ValueType>& manager, std::vector<std::shared_ptr<OpenMdp<ValueType>>> values) : OpenMdp<ValueType>(manager), values(values) {
}

template class SumModel<storm::RationalNumber>;
template class SumModel<double>;

}
}
