#pragma once

#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/models/OpenMdp.h"
#include "storm-compose/models/PrismModel.h"
#include "storm-compose/models/Reference.h"
#include "storm-compose/models/SequenceModel.h"
#include "storm-compose/models/SumModel.h"
#include "storm-compose/models/TraceModel.h"

namespace storm {
namespace models {

template<typename ValueType>
class OpenMdp;
template<typename ValueType>
class PrismModel;
template<typename ValueType>
class ConcreteMdp;
template<typename ValueType>
class SequenceModel;
template<typename ValueType>
class SumModel;
template<typename ValueType>
class Reference;
template<typename ValueType>
class TraceModel;

namespace visitor {

// Unfortunately, templated return and argument types is difficult in C++
// So these functions are void and without argument, which you could pass
// using class member fields.
template<typename ValueType>
class OpenMdpVisitor {
   public:
    virtual ~OpenMdpVisitor() = 0;

    virtual void visitPrismModel(PrismModel<ValueType>& model) {
        // Intentionally left empty
    }

    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) {
        // Intentionally left empty
    }

    virtual void visitReference(Reference<ValueType>& reference) {
        const auto manager = reference.getManager();
        auto dereferenced = manager->dereference(reference.getReference());
        dereferenced->accept(*this);
    }

    virtual void visitSequenceModel(SequenceModel<ValueType>& model) {
        for (const auto& m : model.getValues()) {
            m->accept(*this);
        }
    }

    virtual void visitSumModel(SumModel<ValueType>& model) {
        for (const auto& m : model.getValues()) {
            m->accept(*this);
        }
    }

    virtual void visitTraceModel(TraceModel<ValueType>& model) {
        model.value->accept(*this);
    }
};

template<typename ValueType>
OpenMdpVisitor<ValueType>::~OpenMdpVisitor() {}

}  // namespace visitor
}  // namespace models
}  // namespace storm
