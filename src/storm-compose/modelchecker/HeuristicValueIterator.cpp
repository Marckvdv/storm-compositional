#include "HeuristicValueIterator.h"
#include "exceptions/NotSupportedException.h"
#include "exceptions/OutOfRangeException.h"
#include "solver/SolverSelectionOptions.h"
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
HeuristicValueIterator<ValueType>::HeuristicValueIterator(Options options, std::shared_ptr<models::OpenMdpManager<ValueType>> manager,
                                                          storage::ValueVector<ValueType>& valueVector,
                                                          std::shared_ptr<storage::AbstractCache<ValueType>> cache,
                                                          compose::benchmark::BenchmarkStats<ValueType>& stats)
    : options(options), env(), manager(manager), valueVector(valueVector), mapping(valueVector.getMapping()), cache(cache), stats(stats) {
    // Intentionally left empty
    env.solver().minMax().setMethod(storm::solver::MinMaxMethod::OptimisticValueIteration);
    env.solver().minMax().setPrecision(options.localOviEpsilon);

    if (options.iterationOrder == Options::HEURISTIC) {
        initializeLeafScores();
    }
}

template<typename ValueType>
void HeuristicValueIterator<ValueType>::performIteration() {
    const auto& leaves = mapping.getLeaves();

    if (options.iterationOrder == Options::FORWARD) {
        for (size_t leaf = 0; leaf < leaves.size(); ++leaf) {
            updateModel(leaf);
        }
    } else if (options.iterationOrder == Options::BACKWARD) {
        for (size_t leaf = leaves.size(); leaf-- > 0;) {
            updateModel(leaf);
        }
    } else if (options.iterationOrder == Options::HEURISTIC) {
        for (size_t step = 0; step < options.stepsPerIteration; ++step) {
            size_t leaf = getNextLeaf();
            updateModel(leaf);
        }
    }
}

template<typename ValueType>
typename HeuristicValueIterator<ValueType>::Options::IterationOrder HeuristicValueIterator<ValueType>::orderFromString(std::string const& string) {
    if (string == "forward")
        return Options::IterationOrder::FORWARD;
    else if (string == "backward")
        return Options::IterationOrder::BACKWARD;
    else if (string == "heuristic")
        return Options::IterationOrder::HEURISTIC;
    else
        STORM_LOG_THROW(false, storm::exceptions::NotSupportedException, "Unknown iteration order " << string);
}

template<typename ValueType>
void HeuristicValueIterator<ValueType>::updateModel(size_t leafId) {
    WeightType weights = valueVector.getOutputWeights(leafId);
    WeightType inputWeights = performStep(leafId, weights);

    if (options.iterationOrder == Options::HEURISTIC) {
        updateLeafScores(inputWeights, leafId);
    }
    storeInputWeights(leafId, inputWeights);
}

template<typename ValueType>
void HeuristicValueIterator<ValueType>::storeInputWeights(size_t leafId, WeightType const& inputWeights) {
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
typename HeuristicValueIterator<ValueType>::WeightType HeuristicValueIterator<ValueType>::performStep(size_t leafId, WeightType const& weights) {
    models::ConcreteMdp<ValueType>* model = mapping.getLeaves()[leafId];
    bool allZero = std::all_of(weights.begin(), weights.end(), [](ValueType v) { return v == storm::utility::zero<ValueType>(); });
    std::vector<ValueType> inputWeights;
    if (allZero) {
        inputWeights = std::vector<ValueType>(model->getEntranceCount(), 0);
    } else {
        boost::optional<std::vector<ValueType>> result;
        result = queryCacheLowerBound(model, weights);

        bool cacheUsed = false;
        ++stats.weightedReachabilityQueries;
        if (result) {
            // Check if the result returned by the cache is actually better.
            const auto& result2 = *result;
            auto oldInputWeights = valueVector.getInputWeights(leafId);
            STORM_LOG_ASSERT(oldInputWeights.size() == result2.size(), "size mismatch");

            ValueType cacheTolerance = 1e-3;
            bool goodEnough = false;
            for (size_t i = 0; i < oldInputWeights.size(); ++i) {
                ValueType difference = result2[i] - oldInputWeights[i];
                if (difference > cacheTolerance) {
                    goodEnough = true;
                } else if (difference < 0) {
                    goodEnough = false;
                    break;
                }
            }

            if (goodEnough) {
                inputWeights = result2;
                cacheUsed = true;
                ++stats.cacheHits;
            } else {
                // Check gap
                result = queryCacheLowerBound(model, weights);
            }
        }

        if (!cacheUsed) {
            stats.reachabilityComputationTime.start();
            auto newResult = models::visitor::CVIVisitor<ValueType>::weightedReachability(weights, *model, cache->needScheduler(), env);
            stats.reachabilityComputationTime.stop();
            auto weight = newResult.first;
            auto scheduler = newResult.second;

            inputWeights = weight;
            addToCache(model, weights, inputWeights, scheduler);
        }
    }

    return inputWeights;
}

template<typename ValueType>
size_t HeuristicValueIterator<ValueType>::getNextLeaf() {
    auto largestEntry = reverseLeafScore.rbegin();
    size_t leafId = largestEntry->second;

    ValueType newScore = storm::utility::zero<ValueType>();
    reverseLeafScore.erase(--(largestEntry.base()));
    reverseLeafScore.insert({newScore, leafId});

    leafScore[leafId] = newScore;

    return leafId;
}

template<typename ValueType>
void HeuristicValueIterator<ValueType>::initializeLeafScores() {
    size_t leafCount = mapping.getLeafCount();
    ValueType defaultScore = storm::utility::zero<ValueType>();

    for (size_t leaf = 0; leaf < leafCount; ++leaf) {
        leafScore[leaf] = defaultScore;
        reverseLeafScore.insert({defaultScore, leaf});
    }

    updateLeafScore(leafCount - 1, 1);  // TODO replace with actual exit of interest
}

template<typename ValueType>
void HeuristicValueIterator<ValueType>::updateLeafScore(size_t leafId, ValueType newScore) {
    ValueType oldScore = leafScore[leafId];
    leafScore[leafId] = newScore;

    // TODO perhaps this can be done more efficiently:
    auto range = reverseLeafScore.equal_range(oldScore);
    for (auto i = range.first; i != range.second; ++i) {
        if (i->second == leafId) {
            reverseLeafScore.erase(i);
            break;
        }
    }

    reverseLeafScore.insert({newScore, leafId});
}

template<typename ValueType>
void HeuristicValueIterator<ValueType>::updateLeafScoreIfBigger(size_t leafId, ValueType newScore) {
    // TODO this function can be specialized to reduce the number of lookups performed.
    ValueType oldScore = leafScore[leafId];
    if (newScore > oldScore) {
        updateLeafScore(leafId, newScore);
    }
}

template<typename ValueType>
void HeuristicValueIterator<ValueType>::updateLeafScores(WeightType const& inputWeights, size_t leafId) {
    // TODO implement
    std::unordered_map<size_t, ValueType> scores;
    models::ConcreteMdp<ValueType>* model = mapping.getLeaves()[leafId];

    size_t weightIndex = 0;
    auto processEntrances = [&](size_t entranceCount, storage::EntranceExit entranceExit) {
        for (size_t i = 0; i < entranceCount; ++i) {
            storage::Position outPos{entranceExit, i};
            ValueType oldProbability = valueVector.getWeight(leafId, outPos);
            ValueType newProbability = inputWeights[weightIndex];
            ValueType difference = newProbability - oldProbability;

            STORM_LOG_ASSERT(difference + 1e-6 >= storm::utility::zero<ValueType>(), "update was not monotone, " << oldProbability << " vs " << newProbability);
            auto connectedLeaf = mapping.getConnectedLeafId(leafId, outPos);
            if (connectedLeaf) {
                auto it = scores.find(*connectedLeaf);
                if (it == scores.end()) {
                    scores[*connectedLeaf] = difference;
                } else {
                    scores[*connectedLeaf] += difference;
                }
            }

            ++weightIndex;
        }
    };
    processEntrances(model->getLEntranceCount(), storage::L_ENTRANCE);
    processEntrances(model->getREntranceCount(), storage::R_ENTRANCE);

    for (const auto& entry : scores) {
        updateLeafScoreIfBigger(entry.first, entry.second);
    }
}

template<typename ValueType>
boost::optional<typename HeuristicValueIterator<ValueType>::WeightType> HeuristicValueIterator<ValueType>::queryCacheLowerBound(models::ConcreteMdp<ValueType>* ptr,
                                                                                                                      WeightType outputWeight) {
    stats.cacheRetrievalTime.start();
    auto result = cache->getLowerBound(ptr, outputWeight);
    stats.cacheRetrievalTime.stop();

    return result;
}

template<typename ValueType>
boost::optional<typename HeuristicValueIterator<ValueType>::WeightType> HeuristicValueIterator<ValueType>::queryCacheUpperBound(models::ConcreteMdp<ValueType>* ptr,
                                                                                                                      WeightType outputWeight) {
    stats.cacheRetrievalTime.start();
    auto result = cache->getUpperBound(ptr, outputWeight);
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
