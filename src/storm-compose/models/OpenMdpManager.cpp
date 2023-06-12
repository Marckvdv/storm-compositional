#include "OpenMdpManager.h"

#include "storm/exceptions/UnexpectedException.h"
#include "storm/utility/macros.h"


namespace storm {
namespace models {

template<typename ValueType>
OpenMdpManager<ValueType>::OpenMdpManager() {
}

template<typename ValueType>
std::shared_ptr<OpenMdp<ValueType>> OpenMdpManager<ValueType>::dereference(const std::string& name) {
    return references.at(name);
}

template<typename ValueType>
void OpenMdpManager<ValueType>::setRoot(std::shared_ptr<OpenMdp<ValueType>> root) {
    this->root = root;
}

template<typename ValueType>
void OpenMdpManager<ValueType>::setReference(const std::string& name, std::shared_ptr<OpenMdp<ValueType>> reference) {
    references[name] = reference;
}

template<typename ValueType>
void OpenMdpManager<ValueType>::addReference(const std::string& name, std::shared_ptr<OpenMdp<ValueType>> reference) {
    STORM_LOG_THROW(references.count(name) == 0, storm::exceptions::UnexpectedException, "reference already present");
    setReference(name, reference);
}

template class OpenMdpManager<storm::RationalNumber>;
template class OpenMdpManager<double>;

}
}
