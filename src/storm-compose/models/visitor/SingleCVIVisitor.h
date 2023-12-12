#pragma once

#include "OpenMdpVisitor.h"
#include "storage/Scheduler.h"
#include "storm-compose/modelchecker/CompositionalValueIteration.h"
#include "storm-compose/storage/AbstractCache.h"
#include "storm/environment/Environment.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
class SingleCVIVisitor : public OpenMdpVisitor<ValueType> {
    typedef std::vector<ValueType> WeightType;

   public:
    SingleCVIVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager, CompositionalValueVector<ValueType>& valueVector,
                     std::shared_ptr<storm::storage::AbstractCache<ValueType>> cache);
    virtual ~SingleCVIVisitor();

    void visitPrismModel(PrismModel<ValueType>& model) override;
    void visitConcreteModel(ConcreteMdp<ValueType>& model) override;
    // void visitReference(Reference<ValueType>& reference) override;
    void visitSequenceModel(SequenceModel<ValueType>& model) override;
    void visitSumModel(SumModel<ValueType>& model) override;
    // void visitTraceModel(TraceModel<ValueType>& model) override;

    void updateComponent(/* TODO figure out arguments */);

   private:
    void reset();

    storm::Environment env;
    std::shared_ptr<OpenMdpManager<ValueType>> manager;
    CompositionalValueVector<ValueType>& valueVector;
    std::shared_ptr<storm::storage::AbstractCache<ValueType>> cache;

    size_t currentLeftPosition = 0, currentRightPosition = 0;
    size_t currentLeftExitPosition = 0, currentRightExitPosition = 0;
    size_t currentSequencePosition = 0;
    Scope currentScope;
};

}  // namespace visitor
}  // namespace models
}  // namespace storm
