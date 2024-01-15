#include "BottomUpTermination.h"
#include "exceptions/UnexpectedException.h"
#include "storm-compose/benchmark/BenchmarkStats.h"
#include "storm-compose/modelchecker/ApproximateReachabilityResult.h"
#include "storm-compose/modelchecker/MonolithicOpenMdpChecker.h"
#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/models/visitor/LeafTransformer.h"
#include "storm-compose/models/visitor/LowerUpperParetoVisitor.h"

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
    lowerBounds[&model] = cache.toLowerBoundShortcutMdp(this->manager, &model);
    upperBounds[&model] = cache.toUpperBoundShortcutMdp(this->manager, &model);
}

template<typename ValueType>
storm::modelchecker::ApproximateReachabilityResult<ValueType> BottomUpTermination<ValueType>::getReachabilityResult(
    storm::modelchecker::OpenMdpReachabilityTask task, storm::models::OpenMdp<ValueType>& openMdp) {

    stats.bottomUpTerminationTime.start();
    LeafTransformer<ValueType> lowerBoundTransform([&](ConcreteMdp<ValueType>& concreteMdp) { return *lowerBounds[&concreteMdp]; });
    LeafTransformer<ValueType> upperBoundTransform([&](ConcreteMdp<ValueType>& concreteMdp) { return *upperBounds[&concreteMdp]; });
    openMdp.accept(lowerBoundTransform);
    openMdp.accept(upperBoundTransform);

    auto lowerBoundShortcutMdp = lowerBoundTransform.getResult();
    auto upperBoundShortcutMdp = upperBoundTransform.getResult();

    storm::compose::benchmark::BenchmarkStats<ValueType> stats;  // TODO

    storm::modelchecker::MonolithicOpenMdpChecker<ValueType> lowerChecker(lowerBoundTransform.getManager(), stats);
    storm::modelchecker::MonolithicOpenMdpChecker<ValueType> upperChecker(upperBoundTransform.getManager(), stats);

    auto lowerResult = lowerChecker.check(task);
    auto upperResult = upperChecker.check(task);
    stats.bottomUpTerminationTime.stop();

    return storm::modelchecker::ApproximateReachabilityResult<ValueType>::combineLowerUpper(lowerResult, upperResult);
}

template class BottomUpTermination<double>;
template class BottomUpTermination<storm::RationalNumber>;

}  // namespace visitor
}  // namespace models
}  // namespace storm
