#include "CompositionalValueIteration.h"
#include "storm-compose/modelchecker/ApproximateReachabilityResult.h"
#include "storm-compose/modelchecker/HeuristicValueIterator.h"
#include "storm-compose/modelchecker/OviStepUpdater.h"
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
void CompositionalValueIteration<ValueType>::initializeParetoCurves() {
    // models::visitor::ParetoInitializerVisitor<ValueType> visitor;
    // this->manager->getRoot()->accept(visitor);
}

/*
** Current algorithm structure
** Store 1 global value vector, which indicates the (lower bound of the) optimal value for each input.
** Optimal in the sense of optimizing the (global) input weight.
** Due to the way sum and sequential composition work, many ouputs are also inputs.
**
** In fact, we require that all entrances and exits are either:
** a) an entrance or exit at the root of the string diagram
** b) connected to some other entrance/exit.
**
** We need a mapping from each component's entrance to this value vector.
** Such a mapping maps <component id, entrance id, left entrance> -> size_t
**
** Once we have such a mapping, the model checking procedure goes as follows:
** 1) Initialize the value vector to all zeroes.
** 2) Need to start at the edge of the string diagram as these are the only positions with non-zero values (due to the intial weight vector).
** 3) Given a leaf node, perform standard value iteration with initial weights.
** If there are exits at the edge, the weight is taken from the initial weight.
** For exits that are intermediate, the weight is taken from the global value vector.
** We obtain optimal values for the entrances, which are stored back into the global value vector.
** 4) Repeat. Termination either using the 'optimistic VI' approach or bottom-up TACAS style
*/
template<typename ValueType>
ApproximateReachabilityResult<ValueType> CompositionalValueIteration<ValueType>::check(OpenMdpReachabilityTask task) {
    initialize(task);

    bool oviStop = false, bottomUpStop = false;
    boost::optional<ValueType> lowerBound, upperBound;

    auto noCache = std::make_shared<storage::NoCache<ValueType>>();
    auto root = this->manager->getRoot();

    typename HeuristicValueIterator<ValueType>::Options hviOptions;
    hviOptions.iterationOrder = HeuristicValueIterator<ValueType>::orderFromString(options.iterationOrder);
    hviOptions.localOviEpsilon = options.localOviEpsilon;
    hviOptions.stepsPerIteration = 100;

    HeuristicValueIterator<ValueType> hvi(hviOptions, this->manager, valueVector, cache, this->stats);
    do {
        hvi.performIteration();
        std::cout << "iteration " << currentStep << "/" << options.maxSteps << " current value: " << valueVector.getValues()[0] << std::endl;

        if (shouldCheckOVITermination()) {
            std::cout << std::endl;
            std::cout << "Checking OVI" << std::endl;
            //   Compute v + epsilon
            // std::cout << "Intial VV:" << std::endl;
            // valueVector.print();

            auto newValue = valueVector;
            newValue.addConstant(options.epsilon);
            // auto newValueCopy = newValue;

            // std::cout << "VV + epsilon:" << std::endl;
            // newValue.print();

            std::cout << "Start OVI check!" << std::endl;
            OviStepUpdater<ValueType> upperboundVisitor(hviOptions, this->manager, newValue, cache, this->stats);
            bool terminate = upperboundVisitor.performIteration();
            std::cout << "End OVI check!" << std::endl;

            // std::cout << "Phi(VV + epsilon):" << std::endl;
            // newValue.print();
            // if (newValueCopy.dominates(newValue)) {
            //     // optimistic value iteration stopping criterion
            //     oviStop = true;

            //    lowerBound = valueVector.getValues()[0];  // TODO FIXME don't hardcode this 0
            //    upperBound = utility::min<ValueType>(*lowerBound + options.epsilon + options.localOviEpsilon, utility::one<ValueType>());

            //    break;
            //}
            // std::cout << std::endl;
            if (terminate) {
                // optimistic value iteration stopping criterion
                oviStop = true;

                lowerBound = valueVector.getValues()[0];  // TODO FIXME don't hardcode this 0
                upperBound = utility::min<ValueType>(*lowerBound + options.epsilon + options.localOviEpsilon, utility::one<ValueType>());

                break;
            }
        }

        if (shouldCheckBottomUpTermination()) {
            std::cout << "Checking Bottom-Up" << std::endl;

            // Cache is guaranteed to be a Pareto cache at this point.
            storm::storage::ParetoCache<ValueType>& paretoCache = static_cast<storm::storage::ParetoCache<ValueType>&>(*cache);
            //std::cout << "Cache has " << paretoCache.getLowerParetoPointCount() << " lower pareto points, and" << std::endl;
            //std::cout << paretoCache.getUpperParetoPointCount() << " upper pareto points" << std::endl;

            models::visitor::BottomUpTermination<ValueType> bottomUpVisitor(this->manager, this->stats, env, paretoCache);
            root->accept(bottomUpVisitor);
            auto result = bottomUpVisitor.getReachabilityResult(task, *root);

            if (result.getError() < options.epsilon) {
                bottomUpStop = true;

                lowerBound = result.getLowerBound();
                upperBound = result.getUpperBound();

                break;
            }
        }

        ++currentStep;
    } while (!shouldTerminate());

    std::cout << "WARN for now assuming we are only interested in the first left entrance" << std::endl;
    std::cout << "WARN assuming first entrance is the one of interest" << std::endl;

    if (oviStop) {
        std::cout << "OVI stopping criterion hit" << std::endl;
    } else if (bottomUpStop) {
        std::cout << "Bottom-up stopping criterion hit" << std::endl;
    } else {
        std::cout << "Neither stopping criterion hit" << std::endl;
        lowerBound = valueVector.getValues()[0];  // TODO FIXME don't hardcode this 0
    }

    if (lowerBound && upperBound) {
        return ApproximateReachabilityResult<ValueType>(*lowerBound, *upperBound);
    } else if (lowerBound) {
        STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "No upper bound produced (lower bound=" << *lowerBound << ")");
    } else {
        STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "No upper or lower bound produced");
    }
}

template<typename ValueType>
void CompositionalValueIteration<ValueType>::initialize(OpenMdpReachabilityTask const& task) {
    initializeParetoCurves();
    initializeCache();

    this->stats.modelBuildingTime.start();
    this->manager->constructConcreteMdps();
    this->stats.modelBuildingTime.stop();

    auto root = this->manager->getRoot();

    models::visitor::EntranceExitMappingVisitor<ValueType> entranceExitMappingVisitor;
    root->accept(entranceExitMappingVisitor);

    // TODO FIXME can be retrieved from mapping visitor above
    models::visitor::EntranceExitVisitor<ValueType> entranceExitVisitor;
    entranceExitVisitor.setEntranceExit(storage::L_EXIT);
    root->accept(entranceExitVisitor);
    size_t lOuterExitCount = entranceExitVisitor.getCollected().size();

    entranceExitVisitor.setEntranceExit(storage::R_EXIT);
    root->accept(entranceExitVisitor);
    size_t rOuterExitCount = entranceExitVisitor.getCollected().size();

    models::visitor::MappingVisitor<ValueType> mappingVisitor;
    root->accept(mappingVisitor);
    mappingVisitor.performPostProcessing();
    auto mapping = mappingVisitor.getMapping();
    // std::cout << "M: " << mapping.size() << std::endl;
    // mapping.print();

    auto finalWeight = task.toExitWeights<ValueType>(lOuterExitCount, rOuterExitCount);
    valueVector = storage::ValueVector<ValueType>(std::move(mapping), finalWeight);
    valueVector.initializeValues();
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
