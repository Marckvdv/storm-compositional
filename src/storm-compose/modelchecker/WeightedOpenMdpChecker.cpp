#include "WeightedOpenMdpChecker.h"

#include "storm-compose/models/visitor/PropertyDrivenVisitor.h"

namespace storm {
namespace modelchecker {

template <typename ValueType>
WeightedOpenMdpChecker<ValueType>::WeightedOpenMdpChecker(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager)
    : AbstractOpenMdpChecker<ValueType>(manager) {

}

template <typename ValueType>
ApproximateReachabilityResult<ValueType> WeightedOpenMdpChecker<ValueType>::check(OpenMdpReachabilityTask task) {
    

    typename storm::models::OpenMdp<ValueType>::Scope emptyScope;

    this->manager->constructConcreteMdps();
    auto exits = this->manager->getRoot()->collectEntranceExit(storm::models::OpenMdp<ValueType>::R_EXIT, emptyScope);

    storm::models::visitor::PropertyDrivenVisitor<ValueType> propertyDrivenVisitor(this->manager);
    STORM_LOG_ASSERT(!task.isLeftExit(), "must be right exit");
    propertyDrivenVisitor.setTargetExit(exits.size(), task.getExitId(), false);

    this->manager->getRoot()->accept(propertyDrivenVisitor);

    auto currentWeight = propertyDrivenVisitor.getCurrentWeight();
    std::cout << "final weight vector: " << std::endl;
    for (const auto& v : currentWeight) {
        std::cout << v << std::endl;
    }

    STORM_LOG_ASSERT(task.isLeftEntrance(), "must be left entrance");
    ApproximateReachabilityResult<ValueType> result { currentWeight[task.getEntranceId()], 1 };

    return result;
}

template class WeightedOpenMdpChecker<storm::RationalNumber>;
template class WeightedOpenMdpChecker<double>;

}
}