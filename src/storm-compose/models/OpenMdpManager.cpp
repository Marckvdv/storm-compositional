#include "OpenMdpManager.h"

#include "storm/exceptions/UnexpectedException.h"
#include "storm/utility/macros.h"
#include <memory>
#include "PrismModel.h"

namespace storm {
namespace models {

template<typename ValueType>
OpenMdpManager<ValueType>::OpenMdpManager() {
}

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
}

template<typename ValueType>
void OpenMdpManager<ValueType>::addReference(const std::string& name, std::shared_ptr<OpenMdp<ValueType>> reference) {
    STORM_LOG_THROW(references.count(name) == 0, storm::exceptions::UnexpectedException, "reference already present");
    setReference(name, reference);
}

template<typename ValueType>
void OpenMdpManager<ValueType>::constructConcreteMdps() {
    // TODO this is not sufficient when Prism models are not directly assigned to a reference
    for (auto& entry : references) {
        auto openMdp = entry.second;
        std::cout << "model " << entry.first << std::endl;
        if (openMdp->isPrismModel()) {
            auto prismModel = std::dynamic_pointer_cast<storm::models::PrismModel<ValueType>>(openMdp);
            auto concreteMdp = prismModel->toConcreteMdp();
            setReference(entry.first, std::make_shared<ConcreteMdp<ValueType>>(concreteMdp));
        }
    }
}

template<typename ValueType>
storm::models::sparse::Mdp<ValueType> constructFlatMdp() {
    // How to connect entrances and exits?  Construct iterators that can iterate
    // over a OpenMdp's exits and entrances.  I.e., a Sequence only exposes the
    // left of the first MDP and the right of the last MDP A Sum exposes
    // entrances and exits consecutively.  Once we have these iterators, for a
    // sequence operation we can simply connect all right exits of the first MDP
    // with all the left entrances of the second MDP, in a zipped manner.  The
    // iterator should return a reference to the concrete MDP together with the
    // state index inside the concrete MDP for the given exit/entrance.
    //
    // Alternatively, do not use iterators and simply collect all the required
    // entrances/exits in one sweep.
}

template class OpenMdpManager<storm::RationalNumber>;
template class OpenMdpManager<double>;

}
}
