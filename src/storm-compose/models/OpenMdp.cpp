#include "OpenMdp.h"

#include "Reference.h"

namespace storm {
namespace models {

template<typename ValueType>
OpenMdp<ValueType>::~OpenMdp() {
}

template<typename ValueType>
OpenMdp<ValueType>::OpenMdp(OpenMdpManager<ValueType> &manager) : manager(manager) {
}

template<typename ValueType>
bool OpenMdp<ValueType>::hasName() {
    return name != boost::none;
}

template<typename ValueType>
std::string OpenMdp<ValueType>::getName() {
    return *name;
}

template<typename ValueType>
std::shared_ptr<OpenMdp<ValueType>> OpenMdp<ValueType>::followReferences() {
    if (isReference()) {
        Reference<ValueType>* reference = dynamic_cast<Reference<ValueType>*>(this);
        std::string name = reference->getReference();
        return manager.dereference(name)->followReferences();
    }
    return this->shared_from_this();
}

template<typename ValueType>
OpenMdpManager<ValueType>& OpenMdp<ValueType>::getManager() {
    return manager;
}

template<typename ValueType>
bool OpenMdp<ValueType>::isConcreteMdp() {
    return false;
}

template<typename ValueType>
bool OpenMdp<ValueType>::isSum() {
    return false;
}

template<typename ValueType>
bool OpenMdp<ValueType>::isSequence() {
    return false;
}

template<typename ValueType>
bool OpenMdp<ValueType>::isTrace() {
    return false;
}

template<typename ValueType>
bool OpenMdp<ValueType>::isReference() {
    return false;
}

template<typename ValueType>
bool OpenMdp<ValueType>::isPrismModel() {
    return false;
}

template<typename ValueType>
std::shared_ptr<OpenMdp<ValueType>> OpenMdp<ValueType>::toOpenMdp() {
    return std::dynamic_pointer_cast<OpenMdp<ValueType>>(this->shared_from_this());
}

}
}
