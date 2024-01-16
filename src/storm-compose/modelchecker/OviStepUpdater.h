#pragma once

#include <queue>

#include "storm-compose/benchmark/BenchmarkStats.h"
#include "storm-compose/modelchecker/CompositionalValueIteration.h"
#include "storm-compose/modelchecker/HeuristicValueIterator.h"
#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/storage/AbstractCache.h"
#include "storm-compose/storage/ValueVectorMapping.h"
#include "storm/environment/Environment.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
class OviStepUpdater {
    typedef std::vector<ValueType> WeightType;

   public:
    OviStepUpdater(typename HeuristicValueIterator<ValueType>::Options options, std::shared_ptr<models::OpenMdpManager<ValueType>> manager,
                   storage::ValueVector<ValueType>& valueVector, std::shared_ptr<storage::AbstractCache<ValueType>> cache,
                   compose::benchmark::BenchmarkStats<ValueType>& stats);

    bool performIteration();

   private:
    boost::optional<WeightType> queryCache(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight);
    void updateModel(size_t leafId);
    void addToCache(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight, WeightType inputWeight,
                    boost::optional<storm::storage::Scheduler<ValueType>> sched = boost::none);
    void storeInputWeights(size_t leafId, WeightType const& inputWeights);
    WeightType performStep(size_t leafId, WeightType const& weights);

    typename HeuristicValueIterator<ValueType>::Options options;
    storm::Environment env;
    std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager;
    storage::ValueVector<ValueType> originalValueVector;
    storage::ValueVector<ValueType>& valueVector;
    storage::ValueVectorMapping<ValueType> mapping;
    std::shared_ptr<storm::storage::AbstractCache<ValueType>> cache;
    compose::benchmark::BenchmarkStats<ValueType>& stats;
};

}  // namespace modelchecker
}  // namespace storm
