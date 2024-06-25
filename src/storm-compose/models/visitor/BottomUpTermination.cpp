#include "BottomUpTermination.h"
#include "exceptions/UnexpectedException.h"
#include "storm-compose/benchmark/BenchmarkStats.h"
#include "storm-compose/modelchecker/ApproximateReachabilityResult.h"
#include "storm-compose/modelchecker/MonolithicOpenMdpChecker.h"
#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/models/visitor/LeafTransformer.h"
#include "storm-compose/models/visitor/LowerUpperParetoVisitor.h"
#include "utility/Stopwatch.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
BottomUpTermination<ValueType>::BottomUpTermination(std::shared_ptr<OpenMdpManager<ValueType>> manager,
                                                    storm::compose::benchmark::BenchmarkStats<ValueType>& stats, storm::Environment const& env,
                                                    storm::storage::ParetoCache<ValueType>& cache)
    : env(env), manager(manager), cache(cache), stats(stats) {}

template<typename ValueType>
void BottomUpTermination<ValueType>::visitPrismModel(PrismModel<ValueType>& model) {
    STORM_LOG_THROW(false, storm::exceptions::UnexpectedException, "Expected concrete MDPs");
}

template<typename ValueType>
void BottomUpTermination<ValueType>::visitConcreteModel(ConcreteMdp<ValueType>& model) {
    storm::utility::Stopwatch shortcutTimer;

    stats.shortcutMdpConstructionTime.start();
    lowerBounds[&model] = cache.toLowerBoundShortcutMdp(this->manager, &model);
    upperBounds[&model] = cache.toUpperBoundShortcutMdp(this->manager, &model);
    stats.shortcutMdpConstructionTime.stop();
}

// template<typename ValueType>
// storm::modelchecker::ApproximateReachabilityResult<ValueType> BottomUpTermination<ValueType>::getReachabilityResult(
//     storm::modelchecker::OpenMdpReachabilityTask task, storm::models::OpenMdp<ValueType>& openMdp) {
//
//     stats.bottomUpTerminationTime.start();
//     LeafTransformer<ValueType> lowerBoundTransform([&](ConcreteMdp<ValueType>& concreteMdp) { return *lowerBounds[&concreteMdp]; });
//     LeafTransformer<ValueType> upperBoundTransform([&](ConcreteMdp<ValueType>& concreteMdp) { return *upperBounds[&concreteMdp]; });
//     openMdp.accept(lowerBoundTransform);
//     openMdp.accept(upperBoundTransform);
//
//     auto lowerBoundShortcutMdp = lowerBoundTransform.getResult();
//     auto upperBoundShortcutMdp = upperBoundTransform.getResult();
//
//     storm::modelchecker::MonolithicOpenMdpChecker<ValueType> lowerChecker(lowerBoundTransform.getManager(), stats);
//     storm::modelchecker::MonolithicOpenMdpChecker<ValueType> upperChecker(upperBoundTransform.getManager(), stats);
//
//     auto lowerResult = lowerChecker.check(task);
//     auto upperResult = upperChecker.check(task);
//     stats.bottomUpTerminationTime.stop();
//
//     return storm::modelchecker::ApproximateReachabilityResult<ValueType>::combineLowerUpper(lowerResult, upperResult);
// }

template<typename ValueType>
storm::modelchecker::ApproximateReachabilityResult<ValueType> BottomUpTermination<ValueType>::getReachabilityResult(
    storm::modelchecker::OpenMdpReachabilityTask task, storm::models::OpenMdp<ValueType>& openMdp) {
    storm::utility::Stopwatch transformTimer, reachabilityTimer;

    transformTimer.start();
    LeafTransformer<ValueType> lowerBoundTransform([&](ConcreteMdp<ValueType>& concreteMdp) {
        // std::cout << "LB count: " << lowerBounds.count(&concreteMdp) << std::endl;
        return *lowerBounds[&concreteMdp];
    });
    LeafTransformer<ValueType> upperBoundTransform([&](ConcreteMdp<ValueType>& concreteMdp) { return *upperBounds[&concreteMdp]; });
    openMdp.accept(lowerBoundTransform);
    openMdp.accept(upperBoundTransform);
    transformTimer.stop();

    reachabilityTimer.start();
    storm::modelchecker::MonolithicOpenMdpChecker<ValueType> lowerChecker(lowerBoundTransform.getManager(), stats);
    storm::modelchecker::MonolithicOpenMdpChecker<ValueType> upperChecker(upperBoundTransform.getManager(), stats);

    auto lowerResult = lowerChecker.check(task);
    auto upperResult = upperChecker.check(task);
    reachabilityTimer.stop();

    std::cout << "Transform time: " << transformTimer.getTimeInNanoseconds() * 1e-9 << std::endl;
    std::cout << "Reachability time: " << reachabilityTimer.getTimeInNanoseconds() * 1e-9 << std::endl;
    std::cout << "(Total) shortcut construction time " << stats.shortcutMdpConstructionTime.getTimeInNanoseconds() * 1e-9 << std::endl;
    // updateParetoStats();

    return storm::modelchecker::ApproximateReachabilityResult<ValueType>::combineLowerUpper(lowerResult, upperResult);
}

template<typename ValueType>
void BottomUpTermination<ValueType>::updateParetoStats() {
    stats.lowerParetoPoints = cache.getLowerParetoPointCount();
    stats.upperParetoPoints = cache.getUpperParetoPointCount();
}

template class BottomUpTermination<double>;
template class BottomUpTermination<storm::RationalNumber>;

}  // namespace visitor
}  // namespace models
}  // namespace storm