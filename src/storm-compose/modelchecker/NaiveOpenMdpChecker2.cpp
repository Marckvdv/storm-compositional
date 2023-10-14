#include "NaiveOpenMdpChecker2.h"

#include "storm-compose/modelchecker/AbstractOpenMdpChecker.h"
#include "storm-compose/models/visitor/LowerUpperParetoVisitor.h"

namespace storm {
namespace modelchecker {

template <typename ValueType>
NaiveOpenMdpChecker2<ValueType>::NaiveOpenMdpChecker2(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager, storm::compose::benchmark::BenchmarkStats<ValueType>& stats) : AbstractOpenMdpChecker<ValueType>(manager, stats) {

}

template <typename ValueType>
ApproximateReachabilityResult<ValueType> NaiveOpenMdpChecker2<ValueType>::check(OpenMdpReachabilityTask task) {
    storm::models::visitor::LowerUpperParetoVisitor<ValueType> paretoVisitor(this->manager, this->stats);
    this->stats.modelBuildingTime.start();
    this->manager->constructConcreteMdps();
    this->stats.modelBuildingTime.stop();
    this->manager->getRoot()->accept(paretoVisitor);

    auto currentPareto = paretoVisitor.getCurrentPareto();

    //STORM_LOG_ASSERT(currentPareto.hasEntrance(task.getEntranceId(), task.isLeftEntrance()), "Pareto curve not defined for the requested entrance");
    //std::cout << "lower pareto: " << std::endl << currentPareto.first << std::endl;
    //std::cout << "upper pareto: " << std::endl << currentPareto.second << std::endl;

    // TODO check that below is correct
    // TODO replace with: find point on polytope that maximizes the weight vector indicated by the chosen exit.
    // TODO actually use the ParetoCurveCheckResult to store pareto curves OR new class that stores the over and under approximation polytopes.
    ValueType lowerBound = currentPareto.first.getLowerBound(task.getEntranceId(), task.isLeftEntrance(), task.getExitId(), task.isLeftExit());
    ValueType upperBound = currentPareto.second.getLowerBound(task.getEntranceId(), task.isLeftEntrance(), task.getExitId(), task.isLeftExit());
    //return ApproximateReachabilityResult<ValueType>(lowerBound, storm::utility::one<ValueType>());
    std::cout << lowerBound << " <= p <= " << upperBound << std::endl;
    return ApproximateReachabilityResult<ValueType>(lowerBound, upperBound);
}

template class NaiveOpenMdpChecker2<storm::RationalNumber>;
template class NaiveOpenMdpChecker2<double>;

}
}