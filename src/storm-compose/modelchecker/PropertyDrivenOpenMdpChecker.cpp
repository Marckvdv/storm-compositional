#include "PropertyDrivenOpenMdpChecker.h"
#include "storm-compose/models/visitor/ParetoInitializerVisitor.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
PropertyDrivenOpenMdpChecker<ValueType>::PropertyDrivenOpenMdpChecker(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager,
                                                                      storm::compose::benchmark::BenchmarkStats<ValueType>& stats)
    : AbstractOpenMdpChecker<ValueType>(manager, stats) {}

template<typename ValueType>
void PropertyDrivenOpenMdpChecker<ValueType>::initializeParetoCurves() {
    models::visitor::ParetoInitializerVisitor<ValueType> visitor;
    this->manager->getRoot()->accept(visitor);
}

template<typename ValueType>
ApproximateReachabilityResult<ValueType> PropertyDrivenOpenMdpChecker<ValueType>::check(OpenMdpReachabilityTask task) {
    initializeParetoCurves();
    do {
        auto rootPareto = paretoCurves.at(this->manager->getRoot()->getName());

        //} while(distance between rootPareto.lower and rootPareto.upper is smaller than epsilon);
    } while (1);
}

template<typename ValueType>
std::vector<ValueType> PropertyDrivenOpenMdpChecker<ValueType>::getTargetWeight(OpenMdpReachabilityTask const& task) const {}

template class PropertyDrivenOpenMdpChecker<storm::RationalNumber>;
template class PropertyDrivenOpenMdpChecker<double>;

}  // namespace modelchecker
}  // namespace storm
