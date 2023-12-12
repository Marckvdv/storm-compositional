#pragma once

#include "EntranceExitVisitor.h"
#include "OpenMdpVisitor.h"
#include "exceptions/InvalidOperationException.h"
#include "storm-compose/models/PrismModel.h"
#include "storm-compose/models/Reference.h"
#include "storm-compose/models/SumModel.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
class EntranceExitMappingVisitor : public OpenMdpVisitor<ValueType> {
   public:
    EntranceExitMappingVisitor() {}
    virtual ~EntranceExitMappingVisitor() override {}

    virtual void visitPrismModel(PrismModel<ValueType>& model) override;
    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override;
    virtual void visitReference(Reference<ValueType>& reference) override;
    virtual void visitSequenceModel(SequenceModel<ValueType>& model) override;
    virtual void visitSumModel(SumModel<ValueType>& model) override;
    virtual void visitTraceModel(TraceModel<ValueType>& model) override;

    std::map<std::pair<Scope, storage::Position>, size_t>& getScopeMapping();

   private:
    std::map<std::pair<Scope, storage::Position>, size_t> scopeMapping;

    Scope currentScope;

    size_t currentLeftPosition = 0, currentRightPosition = 0;
    size_t currentSequencePosition = 0;
};

template class EntranceExitMappingVisitor<double>;
template class EntranceExitMappingVisitor<storm::RationalNumber>;

}  // namespace visitor
}  // namespace models
}  // namespace storm
