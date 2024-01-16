#include "NaiveOpenMdpChecker.h"

#include "storm-compose/modelchecker/AbstractOpenMdpChecker.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
NaiveOpenMdpChecker<ValueType>::NaiveOpenMdpChecker(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager,
                                                    storm::compose::benchmark::BenchmarkStats<ValueType>& stats,
                                                    models::visitor::LowerUpperParetoSettings settings)
    : AbstractOpenMdpChecker<ValueType>(manager, stats), settings(settings) {}

template<typename ValueType>
ApproximateReachabilityResult<ValueType> NaiveOpenMdpChecker<ValueType>::check(OpenMdpReachabilityTask task) {
    storm::models::visitor::LowerUpperParetoVisitor<ValueType> paretoVisitor(this->manager, this->stats, settings);
    this->stats.modelBuildingTime.start();
    this->manager->constructConcreteMdps();
    this->stats.modelBuildingTime.stop();
    this->manager->getRoot()->accept(paretoVisitor);

    auto currentPareto = paretoVisitor.getCurrentPareto();

    ValueType lowerBound = currentPareto.first.getLowerBound(task.getEntranceId(), task.isLeftEntrance(), task.getExitId(), task.isLeftExit());
    ValueType upperBound = currentPareto.second.getLowerBound(task.getEntranceId(), task.isLeftEntrance(), task.getExitId(), task.isLeftExit());
    // return ApproximateReachabilityResult<ValueType>(lowerBound, storm::utility::one<ValueType>());
    std::cout << lowerBound << " <= p <= " << upperBound << std::endl;
    return ApproximateReachabilityResult<ValueType>(lowerBound, upperBound);
}

template class NaiveOpenMdpChecker<storm::RationalNumber>;
template class NaiveOpenMdpChecker<double>;

}  // namespace modelchecker
}  // namespace storm
