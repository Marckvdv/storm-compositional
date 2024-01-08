#pragma once

#include "storm-compose/benchmark/BenchmarkStats.h"
#include "storm-compose/modelchecker/CompositionalValueIteration.h"
#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/storage/AbstractCache.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
class HeuristicValueIterator {
   public:
    HeuristicValueIterator(std::shared_ptr<models::OpenMdpManager<ValueType>> manager, models::visitor::ValueVector<ValueType>& valueVector,
                           std::shared_ptr<storage::AbstractCache<ValueType>> cache, compose::benchmark::BenchmarkStats<ValueType>& stats);

    void performIteration();

   private:
    void updateModel(models::ConcreteMdp<ValueType>* model);

    std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager;
    models::visitor::ValueVector<ValueType>& valueVector;
    std::shared_ptr<storm::storage::AbstractCache<ValueType>> cache;
    compose::benchmark::BenchmarkStats<ValueType>& stats;
};

}  // namespace modelchecker
}  // namespace storm
