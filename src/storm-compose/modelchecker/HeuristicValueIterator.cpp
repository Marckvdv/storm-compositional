#include "HeuristicValueIterator.h"
#include "solver/SolverSelectionOptions.h"
#include "storm-compose/models/visitor/CVIVisitor.h"
#include "storm/environment/solver/MinMaxSolverEnvironment.h"
#include "storm/environment/solver/SolverEnvironment.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
HeuristicValueIterator<ValueType>::HeuristicValueIterator(std::shared_ptr<models::OpenMdpManager<ValueType>> manager,
                                                          storage::ValueVector<ValueType>& valueVector,
                                                          std::shared_ptr<storage::AbstractCache<ValueType>> cache,
                                                          compose::benchmark::BenchmarkStats<ValueType>& stats)
    : env(), manager(manager), valueVector(valueVector), mapping(valueVector.getMapping()), cache(cache), stats(stats) {
    // Intentionally left empty
    env.solver().minMax().setMethod(storm::solver::MinMaxMethod::OptimisticValueIteration);
}

template<typename ValueType>
void HeuristicValueIterator<ValueType>::performIteration() {
    const auto& leaves = mapping.getLeaves();

    // forward
    // for (size_t leaf = 0; leaf < leaves.size(); ++leaf) {
    //    updateModel(leaf);
    //}
    // backward
    for (size_t leaf = leaves.size(); leaf-- > 0;) {
        updateModel(leaf);
    }
}

template<typename ValueType>
void HeuristicValueIterator<ValueType>::updateModel(size_t leafId) {
    models::ConcreteMdp<ValueType>* model = mapping.getLeaves()[leafId];
    std::vector<ValueType> weights = valueVector.getOutputWeights(leafId);
    bool allZero = std::all_of(weights.begin(), weights.end(), [](ValueType v) { return v == storm::utility::zero<ValueType>(); });

    std::vector<ValueType> inputValues;
    if (allZero) {
        inputValues = std::vector<ValueType>(model->getEntranceCount(), 0);
    } else {
        boost::optional<std::vector<ValueType>> result;
        result = queryCache(model, weights);

        ++stats.weightedReachabilityQueries;
        if (result) {
            inputValues = *result;
            ++stats.cacheHits;
        } else {
            stats.reachabilityComputationTime.start();
            auto newResult = models::visitor::CVIVisitor<ValueType>::weightedReachability(weights, *model, cache->needScheduler(), env);
            stats.reachabilityComputationTime.stop();
            auto weight = newResult.first;
            auto scheduler = newResult.second;

            inputValues = weight;
            addToCache(model, weights, inputValues, scheduler);
        }
    }

    size_t weightIndex = 0;
    for (size_t i = 0; i < model->getLEntrance().size(); ++i) {
        std::pair<storage::EntranceExit, size_t> pos{storage::L_ENTRANCE, i};
        valueVector.setWeight(leafId, pos, inputValues[weightIndex]);

        ++weightIndex;
    }

    for (size_t i = 0; i < model->getREntrance().size(); ++i) {
        std::pair<storage::EntranceExit, size_t> pos{storage::R_ENTRANCE, i};
        valueVector.setWeight(leafId, pos, inputValues[weightIndex]);

        ++weightIndex;
    }
}

template<typename ValueType>
boost::optional<typename HeuristicValueIterator<ValueType>::WeightType> HeuristicValueIterator<ValueType>::queryCache(models::ConcreteMdp<ValueType>* ptr,
                                                                                                                      WeightType outputWeight) {
    stats.cacheRetrievalTime.start();
    auto result = cache->getLowerBound(ptr, outputWeight);
    stats.cacheRetrievalTime.stop();

    return result;
}

template<typename ValueType>
void HeuristicValueIterator<ValueType>::addToCache(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight, WeightType inputWeight,
                                                   boost::optional<storm::storage::Scheduler<ValueType>> sched) {
    stats.cacheInsertionTime.start();
    cache->addToCache(ptr, outputWeight, inputWeight, sched);
    stats.cacheInsertionTime.stop();
}

template class HeuristicValueIterator<double>;
template class HeuristicValueIterator<storm::RationalNumber>;

}  // namespace modelchecker
}  // namespace storm
