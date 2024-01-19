#include "CompositionalValueIteration.h"
#include "storm-compose/modelchecker/ApproximateReachabilityResult.h"
#include "storm-compose/modelchecker/HeuristicValueIterator.h"
#include "storm-compose/modelchecker/OviStepUpdater.h"
#include "storm-compose/modelchecker/ProperOviTermination.h"
#include "storm-compose/models/visitor/BottomUpTermination.h"
#include "storm-compose/models/visitor/EntranceExitMappingVisitor.h"
#include "storm-compose/models/visitor/EntranceExitVisitor.h"
#include "storm-compose/models/visitor/MappingVisitor.h"
#include "storm-compose/models/visitor/OuterEntranceExitVisitor.h"
#include "storm-compose/models/visitor/ParetoInitializerVisitor.h"
#include "storm-compose/storage/EntranceExit.h"
#include "storm-compose/storage/ExactCache.h"
#include "storm-compose/storage/NoCache.h"
#include "storm-compose/storage/ParetoCache.h"
#include "storm/exceptions/InvalidOperationException.h"
#include "storm/exceptions/NotSupportedException.h"
#include "storm/utility/SignalHandler.h"
#include "storm/utility/constants.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
CompositionalValueIteration<ValueType>::CompositionalValueIteration(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager,
                                                                    storm::compose::benchmark::BenchmarkStats<ValueType>& stats, Options options)
    : AbstractOpenMdpChecker<ValueType>(manager, stats), options(options), env() {
    STORM_LOG_WARN_COND(!(options.useOvi && options.maxSteps % options.oviInterval != 0), "OVI interval not a multiple of max steps");
    STORM_LOG_WARN_COND(!(options.useBottomUp && options.maxSteps % options.bottomUpInterval != 0), "Bottom-up interval not a multiple of max steps");
}

template<typename ValueType>
ApproximateReachabilityResult<ValueType> CompositionalValueIteration<ValueType>::check(OpenMdpReachabilityTask task) {
    initialize(task);

    STORM_LOG_THROW(!options.useOvi || !options.useBottomUp, storm::exceptions::NotSupportedException, "Using OVI and bottom-up together is not supported");
    STORM_LOG_THROW(options.useOvi || options.useBottomUp, storm::exceptions::NotSupportedException, "Need either OVI or bottom-up termination");

    if (options.useOvi) {
        return checkOvi(task);
    } else {
        return checkBottomUp(task);
    }
}

template<typename ValueType>
ApproximateReachabilityResult<ValueType> CompositionalValueIteration<ValueType>::checkOvi(OpenMdpReachabilityTask task) {
    initialize(task);

    boost::optional<ValueType> lowerValue, upperValue;

    // auto exactCache = std::make_shared<storage::ExactCache<ValueType>>();
    auto noCache = std::make_shared<storage::NoCache<ValueType>>();
    auto root = this->manager->getRoot();

    typename HeuristicValueIterator<ValueType>::Options hviOptions;
    hviOptions.iterationOrder = HeuristicValueIterator<ValueType>::orderFromString(options.iterationOrder);
    hviOptions.localOviEpsilon = options.localOviEpsilon;

    typename HeuristicValueIterator<ValueType>::Options oviOptions;
    oviOptions.exactOvi = options.localOviEpsilon == 0;
    HeuristicValueIterator<ValueType> lowerBoundIterator(hviOptions, this->manager, lowerBound, cache, this->stats);

    do {
        lowerBoundIterator.performIteration();
        std::cout << "iteration " << currentStep << "/" << options.maxSteps << " current value: " << lowerBound.getValues()[0] << std::endl;

        if (shouldCheckOVITermination()) {
            std::cout << "Checking OVI" << std::endl;

            upperBound = lowerBound;
            upperBound.addConstant(options.epsilon);
            size_t iter = 0;
            while (upperBound.comparable(lowerBound)) {
                std::cout << "OVI iteration " << iter << std::endl;
                // TODO change caching behaviour:
                // OviStepUpdater<ValueType> oviLowerBoundIterator(oviOptions, this->manager, lowerBound, noCache, this->stats);
                //auto& oviCache = oviOptions.exactOvi ? noCache : cache;
                auto& oviCache = noCache;
                OviStepUpdater<ValueType> upperBoundIterator(oviOptions, this->manager, upperBound, oviCache, this->stats);

                upperBoundIterator.performIteration();
                lowerBoundIterator.performIteration();
                auto newUpperBound = upperBoundIterator.getNewValueVector();

                bool inductiveUpperBound = newUpperBound.dominatedBy(upperBound);
                if (inductiveUpperBound) {
                    // Done
                    lowerValue = lowerBound.getValues()[0];  // TODO FIXME
                    upperValue = upperBound.getValues()[0];  // TODO FIXME
                    goto cviLoopBreak;
                }

                upperBound = newUpperBound;
                ++iter;
            }
        }

        ++currentStep;
    } while (!shouldTerminate());
cviLoopBreak:

    std::cout << "WARN for now assuming we are only interested in the first left entrance" << std::endl;
    std::cout << "WARN assuming first entrance is the one of interest" << std::endl;

    if (lowerValue && upperValue) {
        return ApproximateReachabilityResult<ValueType>(*lowerValue, *upperValue);
    } else if (lowerValue) {
        STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "No upper bound produced (lower bound=" << *lowerValue << ")");
    } else {
        STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "No upper or lower bound produced");
    }
}

template<typename ValueType>
ApproximateReachabilityResult<ValueType> CompositionalValueIteration<ValueType>::checkBottomUp(OpenMdpReachabilityTask task) {
    initialize(task);

    boost::optional<ValueType> lowerValue, upperValue;

    auto noCache = std::make_shared<storage::NoCache<ValueType>>();
    auto root = this->manager->getRoot();

    typename HeuristicValueIterator<ValueType>::Options hviOptions;
    hviOptions.iterationOrder = HeuristicValueIterator<ValueType>::orderFromString(options.iterationOrder);
    hviOptions.localOviEpsilon = options.localOviEpsilon;
    hviOptions.stepsPerIteration = 100;

    HeuristicValueIterator<ValueType> hvi(hviOptions, this->manager, lowerBound, cache, this->stats);
    do {
        hvi.performIteration();
        std::cout << "iteration " << currentStep << "/" << options.maxSteps << " current value: " << lowerBound.getValues()[0] << std::endl;

        if (shouldCheckBottomUpTermination()) {
            std::cout << "Checking Bottom-Up" << std::endl;

            // Cache is guaranteed to be a Pareto cache at this point.
            storm::storage::ParetoCache<ValueType>& paretoCache = static_cast<storm::storage::ParetoCache<ValueType>&>(*cache);
            std::cout << "Cache has " << paretoCache.getLowerParetoPointCount() << " lower pareto points, and" << std::endl;
            std::cout << paretoCache.getUpperParetoPointCount() << " upper pareto points" << std::endl;

            models::visitor::BottomUpTermination<ValueType> bottomUpVisitor(this->manager, this->stats, env, paretoCache);
            root->accept(bottomUpVisitor);
            auto result = bottomUpVisitor.getReachabilityResult(task, *root);

            if (result.getError() < options.epsilon) {
                lowerValue = result.getLowerBound();
                upperValue = result.getUpperBound();

                break;
            }
        }

        ++currentStep;
    } while (!shouldTerminate());

    if (lowerValue && upperValue) {
        return ApproximateReachabilityResult<ValueType>(*lowerValue, *upperValue);
    } else if (lowerValue) {
        STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "No upper bound produced (lower bound=" << *lowerValue << ")");
    } else {
        STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "No upper or lower bound produced");
    }
}

template<typename ValueType>
void CompositionalValueIteration<ValueType>::initialize(OpenMdpReachabilityTask const& task) {
    initializeCache();

    this->stats.modelBuildingTime.start();
    this->manager->constructConcreteMdps();
    this->stats.modelBuildingTime.stop();

    auto root = this->manager->getRoot();

    models::visitor::MappingVisitor<ValueType> mappingVisitor;
    root->accept(mappingVisitor);
    mappingVisitor.performPostProcessing();
    auto mapping = mappingVisitor.getMapping();

    size_t lOuterExitCount = 0;
    size_t rOuterExitCount = 0;
    for (const auto& entry : mapping.getOuterPositions()) {
        if (entry.second.first == storage::L_EXIT)
            ++lOuterExitCount;
        if (entry.second.first == storage::R_EXIT)
            ++rOuterExitCount;
    }

    models::visitor::EntranceExitMappingVisitor<ValueType> entranceExitMappingVisitor;
    root->accept(entranceExitMappingVisitor);

    auto finalWeight = task.toExitWeights<ValueType>(lOuterExitCount, rOuterExitCount);

    lowerBound = storage::ValueVector<ValueType>(mapping, finalWeight);
    lowerBound.initializeValues();
    upperBound = lowerBound;
}

template<typename ValueType>
void CompositionalValueIteration<ValueType>::initializeCache() {
    // Bottom-up apprach requires Pareto cache
    STORM_LOG_THROW(!(options.useBottomUp && options.cacheMethod != PARETO_CACHE), storm::exceptions::NotSupportedException,
                    "Bottom-up termination requires Pareto cache");

    if (options.cacheMethod == NO_CACHE) {
        cache = std::make_shared<storm::storage::NoCache<ValueType>>();
    } else if (options.cacheMethod == EXACT_CACHE) {
        cache = std::make_shared<storm::storage::ExactCache<ValueType>>(options.localOviEpsilon);
    } else if (options.cacheMethod == PARETO_CACHE) {
        cache = std::make_shared<storm::storage::ParetoCache<ValueType>>();
        cache->setErrorTolerance(options.cacheErrorTolerance);
    }
}

template<typename ValueType>
bool CompositionalValueIteration<ValueType>::shouldTerminate() {
    return currentStep > options.maxSteps || storm::utility::resources::isTerminate();
}

template<typename ValueType>
bool CompositionalValueIteration<ValueType>::shouldCheckOVITermination() {
    return options.useOvi && currentStep % options.oviInterval == 0;
}

template<typename ValueType>
bool CompositionalValueIteration<ValueType>::shouldCheckBottomUpTermination() {
    return options.useBottomUp && currentStep % options.bottomUpInterval == 0;
}

template class CompositionalValueIteration<storm::RationalNumber>;
template class CompositionalValueIteration<double>;

}  // namespace modelchecker
}  // namespace storm
