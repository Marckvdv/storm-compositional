#include "OviStepUpdater.h"
#include "exceptions/NotSupportedException.h"
#include "exceptions/OutOfRangeException.h"
#include "solver/SolverSelectionOptions.h"
#include "storm-compose/modelchecker/HeuristicValueIterator.h"
#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/models/visitor/CVIVisitor.h"
#include "storm-compose/storage/EntranceExit.h"
#include "storm/environment/solver/MinMaxSolverEnvironment.h"
#include "storm/environment/solver/SolverEnvironment.h"
#include "utility/constants.h"
#include "utility/macros.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
OviStepUpdater<ValueType>::OviStepUpdater(typename HeuristicValueIterator<ValueType>::Options options,
                                          std::shared_ptr<models::OpenMdpManager<ValueType>> manager, storage::ValueVector<ValueType>& valueVector,
                                          std::shared_ptr<storage::AbstractCache<ValueType>> cache, compose::benchmark::BenchmarkStats<ValueType>& stats)
    : options(options), env(), manager(manager), valueVector(valueVector), mapping(valueVector.getMapping()), cache(cache), stats(stats) {
    // Intentionally left empty
    env.solver().minMax().setMethod(storm::solver::MinMaxMethod::OptimisticValueIteration);
    env.solver().minMax().setPrecision(options.localOviEpsilon);
}

template<typename ValueType>
void OviStepUpdater<ValueType>::performIteration() {
    const auto& leaves = mapping.getLeaves();

    for (size_t leaf = 0; leaf < leaves.size(); ++leaf) {
        updateModel(leaf);
    }
}

template<typename ValueType>
void OviStepUpdater<ValueType>::updateModel(size_t leafId) {
    WeightType weights = valueVector.getOutputWeights(leafId);
    WeightType inputWeights = performStep(leafId, weights);
    storeInputWeights(leafId, inputWeights);
}

template<typename ValueType>
void OviStepUpdater<ValueType>::storeInputWeights(size_t leafId, WeightType const& inputWeights) {
    models::ConcreteMdp<ValueType>* model = mapping.getLeaves()[leafId];
    size_t weightIndex = 0;
    for (size_t i = 0; i < model->getLEntrance().size(); ++i) {
        std::pair<storage::EntranceExit, size_t> pos{storage::L_ENTRANCE, i};
        valueVector.setWeight(leafId, pos, inputWeights[weightIndex]);

        ++weightIndex;
    }

    for (size_t i = 0; i < model->getREntrance().size(); ++i) {
        std::pair<storage::EntranceExit, size_t> pos{storage::R_ENTRANCE, i};
        valueVector.setWeight(leafId, pos, inputWeights[weightIndex]);

        ++weightIndex;
    }
}

template<typename ValueType>
typename OviStepUpdater<ValueType>::WeightType OviStepUpdater<ValueType>::performStep(size_t leafId, WeightType const& weights) {
    models::ConcreteMdp<ValueType>* model = mapping.getLeaves()[leafId];
    bool allZero = std::all_of(weights.begin(), weights.end(), [](ValueType v) { return v == storm::utility::zero<ValueType>(); });
    std::vector<ValueType> inputWeights;
    if (allZero) {
        inputWeights = std::vector<ValueType>(model->getEntranceCount(), 0);
    } else {
        boost::optional<std::vector<ValueType>> result;
        result = queryCache(model, weights);

        ++stats.weightedReachabilityQueries;
        if (result) {
            inputWeights = *result;
            ++stats.cacheHits;
        } else {
            stats.reachabilityComputationTime.start();
            auto newResult = models::visitor::CVIVisitor<ValueType>::weightedReachability(weights, *model, cache->needScheduler(), env);
            stats.reachabilityComputationTime.stop();
            std::vector<ValueType> weight(newResult.first), upperboundWeight(newResult.first);
            for (auto& v : upperboundWeight) {
                v = storm::utility::min<ValueType>(v + options.localOviEpsilon, storm::utility::one<ValueType>());
            }
            auto scheduler = newResult.second;

            inputWeights = upperboundWeight;
            addToCache(model, weights, weight, scheduler);
        }
    }

    return inputWeights;
}

template<typename ValueType>
boost::optional<typename OviStepUpdater<ValueType>::WeightType> OviStepUpdater<ValueType>::queryCache(models::ConcreteMdp<ValueType>* ptr,
                                                                                                      WeightType outputWeight) {
    stats.cacheRetrievalTime.start();
    auto result = cache->getUpperBound(ptr, outputWeight);
    stats.cacheRetrievalTime.stop();

    return result;
}

template<typename ValueType>
void OviStepUpdater<ValueType>::addToCache(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight, WeightType inputWeight,
                                           boost::optional<storm::storage::Scheduler<ValueType>> sched) {
    stats.cacheInsertionTime.start();
    cache->addToCache(ptr, outputWeight, inputWeight, sched);
    stats.cacheInsertionTime.stop();
}

template class OviStepUpdater<double>;
template class OviStepUpdater<storm::RationalNumber>;

}  // namespace modelchecker
}  // namespace storm
