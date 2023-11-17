#include "OpenMdp.h"

#include "Reference.h"

namespace storm {
namespace models {

template<typename ValueType>
OpenMdp<ValueType>::~OpenMdp() {}

template<typename ValueType>
OpenMdp<ValueType>::OpenMdp(std::weak_ptr<OpenMdpManager<ValueType>> manager) : manager(manager) {}

template<typename ValueType>
bool OpenMdp<ValueType>::hasName() {
    return name != boost::none;
}

template<typename ValueType>
std::string OpenMdp<ValueType>::getName() {
    return *name;
}

template<typename ValueType>
void OpenMdp<ValueType>::setName(std::string const& newName) {
    name = newName;
}

template<typename ValueType>
std::shared_ptr<OpenMdp<ValueType>> OpenMdp<ValueType>::followReferences() {
    if (isReference()) {
        Reference<ValueType>* reference = dynamic_cast<Reference<ValueType>*>(this);
        std::string name = reference->getReference();
        return getManager()->dereference(name)->followReferences();
    }
    return this->shared_from_this();
}

template<typename ValueType>
std::shared_ptr<OpenMdpManager<ValueType>> OpenMdp<ValueType>::getManager() {
    return manager.lock();
}

template<typename ValueType>
std::shared_ptr<OpenMdpManager<ValueType>> const OpenMdp<ValueType>::getManager() const {
    return manager.lock();
}

template<typename ValueType>
bool OpenMdp<ValueType>::isConcreteMdp() const {
    return false;
}

template<typename ValueType>
bool OpenMdp<ValueType>::isSum() const {
    return false;
}

template<typename ValueType>
bool OpenMdp<ValueType>::isSequence() const {
    return false;
}

template<typename ValueType>
bool OpenMdp<ValueType>::isTrace() const {
    return false;
}

template<typename ValueType>
bool OpenMdp<ValueType>::isReference() const {
    return false;
}

template<typename ValueType>
bool OpenMdp<ValueType>::isPrismModel() const {
    return false;
}

template<typename ValueType>
std::shared_ptr<OpenMdp<ValueType>> OpenMdp<ValueType>::toOpenMdp() {
    return std::dynamic_pointer_cast<OpenMdp<ValueType>>(this->shared_from_this());
}

template<typename ValueType>
void OpenMdp<ValueType>::initializeParetoCurve() {
    // Scope emptyScope = {};

    // size_t lEntrances = collectEntranceExit(OpenMdp<ValueType>::L_ENTRANCE, emptyScope).size();
    // size_t rEntrances = collectEntranceExit(OpenMdp<ValueType>::R_ENTRANCE, emptyScope).size();
    // size_t lExits = collectEntranceExit(OpenMdp<ValueType>::L_EXIT, emptyScope).size();
    // size_t rExits = collectEntranceExit(OpenMdp<ValueType>::R_EXIT, emptyScope).size();

    // size_t weightVectorDimension = lExits + rExits;

    std::shared_ptr<storm::storage::geometry::Polytope<ValueType>> underApproximation = storm::storage::geometry::Polytope<ValueType>::createEmptyPolytope();
    std::shared_ptr<storm::storage::geometry::Polytope<ValueType>> overApproximation = storm::storage::geometry::Polytope<ValueType>::createUniversalPolytope();
    // TODO replace overApproximation with subdistriubtion polytope.

    paretoCurve = {{underApproximation, overApproximation}};
}

template<typename ValueType>
typename OpenMdp<ValueType>::EntranceExit OpenMdp<ValueType>::match(EntranceExit entranceExit) {
    switch (entranceExit) {
        case OpenMdp<ValueType>::L_ENTRANCE:
            return OpenMdp<ValueType>::R_EXIT;

        case OpenMdp<ValueType>::R_ENTRANCE:
            return OpenMdp<ValueType>::L_EXIT;

        case OpenMdp<ValueType>::L_EXIT:
            return OpenMdp<ValueType>::R_ENTRANCE;

        case OpenMdp<ValueType>::R_EXIT:
            return OpenMdp<ValueType>::L_ENTRANCE;
    }
}

template class OpenMdp<double>;
template class OpenMdp<storm::RationalNumber>;

}  // namespace models
}  // namespace storm
