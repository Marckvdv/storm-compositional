#pragma once

#include "ParetoVisitor.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
class TrueParetoVisitor : public ParetoVisitor<ValueType> {
public:
    TrueParetoVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager);
    //
    //virtual void visitPrismModel(PrismModel<ValueType>& model) override;

    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override;
    virtual void visitReference(Reference<ValueType>& reference) override;
    virtual void visitSequenceModel(SequenceModel<ValueType>& model) override;
    virtual void visitSumModel(SumModel<ValueType>& model) override;
    virtual void visitTraceModel(TraceModel<ValueType>& model) override;

private:
};

}
}
}
