#pragma once

#include <queue>

#include "storm-compose/benchmark/BenchmarkStats.h"
#include "storm-compose/modelchecker/CompositionalValueIteration.h"
#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/storage/AbstractCache.h"
#include "storm-compose/storage/ValueVectorMapping.h"
#include "storm/environment/Environment.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
class HeuristicValueIterator {
    typedef std::vector<ValueType> WeightType;

   public:
    struct Options {
        size_t stepsPerIteration = 100;
        enum IterationOrder { FORWARD, BACKWARD, HEURISTIC } iterationOrder = BACKWARD;
        ValueType localOviEpsilon = 1e-4;
    };

    HeuristicValueIterator(Options options, std::shared_ptr<models::OpenMdpManager<ValueType>> manager, storage::ValueVector<ValueType>& valueVector,
                           std::shared_ptr<storage::AbstractCache<ValueType>> cache, compose::benchmark::BenchmarkStats<ValueType>& stats);

    void performIteration();
    static typename Options::IterationOrder orderFromString(std::string const& string);

   private:
    boost::optional<WeightType> queryCache(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight);
    void updateModel(size_t leafId);
    void addToCache(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight, WeightType inputWeight,
                    boost::optional<storm::storage::Scheduler<ValueType>> sched = boost::none);
    void storeInputWeights(size_t leafId, WeightType const& inputWeights);
    WeightType performStep(size_t leafId, WeightType const& weights);
    size_t getNextLeaf();
    void initializeLeafScores();
    void updateLeafScore(size_t leafId, ValueType newScore);
    void updateLeafScoreIfBigger(size_t leafId, ValueType newScore);
    void updateLeafScores(WeightType const& inputWeights, size_t leafId);

    Options options;
    storm::Environment env;
    std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager;
    storage::ValueVector<ValueType>& valueVector;
    storage::ValueVectorMapping<ValueType> mapping;
    std::shared_ptr<storm::storage::AbstractCache<ValueType>> cache;
    compose::benchmark::BenchmarkStats<ValueType>& stats;

    std::unordered_map<size_t, ValueType> leafScore;
    std::multimap<ValueType, size_t> reverseLeafScore;
};

}  // namespace modelchecker
}  // namespace storm
