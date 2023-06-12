#include "PrismModel.h"

namespace storm {
namespace models {

template<typename ValueType>
PrismModel<ValueType>::PrismModel(OpenMdpManager<ValueType>& manager, std::string path) : OpenMdp<ValueType>(manager), path(path) {

}

template<typename ValueType>
bool PrismModel<ValueType>::isPrismModel() {
    return true;
}

template<typename ValueType>
std::string PrismModel<ValueType>::getPath() {
    return path;
}

template class PrismModel<storm::RationalNumber>;
template class PrismModel<double>;

}
}
