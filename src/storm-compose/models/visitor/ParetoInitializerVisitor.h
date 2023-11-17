#pragma once

#include "OpenMdpVisitor.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
class ParetoInitializerVisitor : public OpenMdpVisitor<ValueType> {
   public:
    ParetoInitializerVisitor() {}

    virtual ~ParetoInitializerVisitor() {}

    virtual void visitPrismModel(PrismModel<ValueType>& model) override {
        model.initializeParetoCurve();
    }

    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override {
        model.initializeParetoCurve();
    }

    virtual void visitSequenceModel(SequenceModel<ValueType>& model) override {
        model.initializeParetoCurve();
        for (auto& v : model.getValues()) {
            v->accept(*this);
        }
    }

    virtual void visitSumModel(SumModel<ValueType>& model) override {
        model.initializeParetoCurve();
        for (auto& v : model.getValues()) {
            v->accept(*this);
        }
    }

    virtual void visitTraceModel(TraceModel<ValueType>& model) override {
        model.initializeParetoCurve();
        model.getValue()->accept(*this);
    }
};

}  // namespace visitor
}  // namespace models
}  // namespace storm
