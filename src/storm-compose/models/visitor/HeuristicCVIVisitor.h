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
class HeuristicCVIVisitor : public OpenMdpVisitor<ValueType> {
    typedef std::vector<ValueType> WeightType;

   public:
    HeuristicCVIVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager, ValueVector<ValueType>& valueVector,
                        std::shared_ptr<storm::storage::AbstractCache<ValueType>> cache, compose::benchmark::BenchmarkStats<ValueType>& stats);
    virtual ~HeuristicCVIVisitor();

   private:
    boost::optional<WeightType> queryCache(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight);
    void addToCache(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight, WeightType inputWeight,
                    boost::optional<storm::storage::Scheduler<ValueType>> sched = boost::none);

    storm::Environment env;
    std::shared_ptr<OpenMdpManager<ValueType>> manager;
    ValueVector<ValueType>& valueVector;
    std::shared_ptr<storm::storage::AbstractCache<ValueType>> cache;
    // Scope currentScope;
    storm::compose::benchmark::BenchmarkStats<ValueType>& stats;
};

}  // namespace visitor
}  // namespace models
}  // namespace storm
