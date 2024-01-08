#pragma once

#include "OpenMdpVisitor.h"
#include "storage/Scheduler.h"
#include "storm-compose/benchmark/BenchmarkStats.h"
#include "storm-compose/modelchecker/CompositionalValueIteration.h"
#include "storm-compose/storage/AbstractCache.h"
#include "storm/environment/Environment.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
class CVIVisitor : public OpenMdpVisitor<ValueType> {
    typedef std::vector<ValueType> WeightType;

   public:
    CVIVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager, ValueVector<ValueType>& valueVector,
               std::shared_ptr<storm::storage::AbstractCache<ValueType>> cache, compose::benchmark::BenchmarkStats<ValueType>& stats);
    virtual ~CVIVisitor();

    void visitPrismModel(PrismModel<ValueType>& model) override;
    void visitConcreteModel(ConcreteMdp<ValueType>& model) override;
    // void visitReference(Reference<ValueType>& reference) override;
    void visitSequenceModel(SequenceModel<ValueType>& model) override;
    void visitSumModel(SumModel<ValueType>& model) override;
    void visitTraceModel(TraceModel<ValueType>& model) override;

    static std::pair<WeightType, boost::optional<storm::storage::Scheduler<ValueType>>> weightedReachability(WeightType weights,
                                                                                                             ConcreteMdp<ValueType> concreteMdp,
                                                                                                             bool returnScheduler, storm::Environment env);
    static std::pair<WeightType, boost::optional<storm::storage::Scheduler<ValueType>>> weightedReachability2(WeightType weights,
                                                                                                              ConcreteMdp<ValueType> concreteMdp,
                                                                                                              bool returnScheduler, storm::Environment env);

   private:
    boost::optional<WeightType> queryCache(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight);
    void addToCache(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight, WeightType inputWeight,
                    boost::optional<storm::storage::Scheduler<ValueType>> sched = boost::none);

    storm::Environment env;
    std::shared_ptr<OpenMdpManager<ValueType>> manager;
    ValueVector<ValueType>& valueVector;
    std::shared_ptr<storm::storage::AbstractCache<ValueType>> cache;

    size_t currentLeftPosition = 0, currentRightPosition = 0;
    size_t currentLeftExitPosition = 0, currentRightExitPosition = 0;
    size_t currentSequencePosition = 0;
    size_t currentLeafId = 0;
    // Scope currentScope;
    storm::compose::benchmark::BenchmarkStats<ValueType>& stats;
};

}  // namespace visitor
}  // namespace models
}  // namespace storm
