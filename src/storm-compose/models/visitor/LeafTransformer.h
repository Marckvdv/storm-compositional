#pragma once

#include <functional>
#include "storm-compose/models/visitor/OpenMdpVisitor.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
class LeafTransformer : public OpenMdpVisitor<ValueType> {
   public:
    LeafTransformer(std::function<ConcreteMdp<ValueType>(ConcreteMdp<ValueType>&)> transformer);

    virtual void visitPrismModel(PrismModel<ValueType>& model) override;
    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override;
    virtual void visitReference(Reference<ValueType>& reference) override;
    virtual void visitSequenceModel(SequenceModel<ValueType>& model) override;
    virtual void visitSumModel(SumModel<ValueType>& model) override;
    virtual void visitTraceModel(TraceModel<ValueType>& model) override;

    std::shared_ptr<OpenMdp<ValueType>> getResult();
    std::shared_ptr<OpenMdpManager<ValueType>> getManager();

   private:
    std::function<ConcreteMdp<ValueType>(ConcreteMdp<ValueType>&)> transformer;
    std::shared_ptr<OpenMdp<ValueType>> result;
    std::shared_ptr<OpenMdpManager<ValueType>> manager;
};

}  // namespace visitor
}  // namespace models
}  // namespace storm
