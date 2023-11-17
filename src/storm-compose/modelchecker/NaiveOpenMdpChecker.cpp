#include "NaiveOpenMdpChecker.h"

#include "storm-compose/modelchecker/AbstractOpenMdpChecker.h"
#include "storm-compose/models/visitor/ParetoVisitor.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
NaiveOpenMdpChecker<ValueType>::NaiveOpenMdpChecker(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager,
                                                    storm::compose::benchmark::BenchmarkStats<ValueType>& stats)
    : AbstractOpenMdpChecker<ValueType>(manager, stats) {}

template<typename ValueType>
ApproximateReachabilityResult<ValueType> NaiveOpenMdpChecker<ValueType>::check(OpenMdpReachabilityTask task) {
    storm::models::visitor::ParetoVisitor<ValueType> paretoVisitor(this->manager);
    this->manager->getRoot()->accept(paretoVisitor);

    auto currentPareto = paretoVisitor.getCurrentPareto();

    STORM_LOG_ASSERT(currentPareto.hasEntrance(task.getEntranceId(), task.isLeftEntrance()), "Pareto curve not defined for the requested entrance");

    // TODO replace with OpenMdpReachabilityTask
    ValueType lowerBound = currentPareto.getLowerBound(task.getEntranceId(), task.isLeftEntrance(), task.getExitId(), task.isLeftExit());
    return ApproximateReachabilityResult<ValueType>(lowerBound, storm::utility::one<ValueType>());
}

template class NaiveOpenMdpChecker<storm::RationalNumber>;
template class NaiveOpenMdpChecker<double>;

}  // namespace modelchecker
}  // namespace storm