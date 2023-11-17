#include "OpenMdpManager.h"

#include <memory>
#include "PrismModel.h"
#include "storm/exceptions/UnexpectedException.h"
#include "storm/utility/macros.h"

namespace storm {
namespace models {

template<typename ValueType>
OpenMdpManager<ValueType>::OpenMdpManager() {}

template<typename ValueType>
std::shared_ptr<OpenMdp<ValueType>> OpenMdpManager<ValueType>::dereference(const std::string& name) const {
    STORM_LOG_THROW(references.count(name) > 0, storm::exceptions::UnexpectedException, "String diagram error: Unknown reference '" << name << "'");
    return references.at(name);
}

template<typename ValueType>
void OpenMdpManager<ValueType>::setRoot(std::shared_ptr<OpenMdp<ValueType>> root) {
    this->root = root;
}

// TODO should return const shared_ptr somehow
template<typename ValueType>
std::shared_ptr<OpenMdp<ValueType>> OpenMdpManager<ValueType>::getRoot() const {
    return root;
}

template<typename ValueType>
void OpenMdpManager<ValueType>::setReference(const std::string& name, std::shared_ptr<OpenMdp<ValueType>> reference) {
    references[name] = reference;
    reference->setName(name);
}

template<typename ValueType>
void OpenMdpManager<ValueType>::addReference(const std::string& name, std::shared_ptr<OpenMdp<ValueType>> reference) {
    STORM_LOG_THROW(references.count(name) == 0, storm::exceptions::UnexpectedException, "reference already present");
    setReference(name, reference);
}

template<typename ValueType>
void OpenMdpManager<ValueType>::constructConcreteMdps() {
    for (auto& entry : references) {
        auto openMdp = entry.second;
        if (openMdp->isPrismModel()) {
            auto prismModel = std::dynamic_pointer_cast<storm::models::PrismModel<ValueType>>(openMdp);
            auto concreteMdp = prismModel->toConcreteMdp();
            setReference(entry.first, std::make_shared<ConcreteMdp<ValueType>>(concreteMdp));
        }
    }
}

template class OpenMdpManager<storm::RationalNumber>;
template class OpenMdpManager<double>;

}  // namespace models
}  // namespace storm
