#pragma once

#include "storm-compose/models/OpenMdp.h"

namespace storm {
namespace models {

template <typename ValueType> class OpenMdp;
template <typename ValueType> class PrismModel;
template <typename ValueType> class ConcreteMdp;
template <typename ValueType> class SequenceModel;
template <typename ValueType> class SumModel;
template <typename ValueType> class Reference;
template <typename ValueType> class TraceModel;

namespace visitor {

// Unfortunately, templated return and argument types is difficult in C++
// So these functions are void and without argument, which you could pass
// using class member fields.
template<typename ValueType>
class OpenMdpVisitor {
    public:
    virtual ~OpenMdpVisitor() = 0;

    virtual void visitPrismModel(const PrismModel<ValueType>& model) {}
    virtual void visitConcreteModel(const ConcreteMdp<ValueType>& model) {}
    virtual void visitReference(const Reference<ValueType>& reference) {}
    virtual void visitSequenceModel(const SequenceModel<ValueType>& model) {}
    virtual void visitSumModel(const SumModel<ValueType>& model) {}
    virtual void visitTraceModel(const TraceModel<ValueType>& model) {}
};

template<typename ValueType>
OpenMdpVisitor<ValueType>::~OpenMdpVisitor() {
}

}
}
}
