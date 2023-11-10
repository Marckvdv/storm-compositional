#include "PropertyDrivenOpenMdpChecker.h"

#include "storm-compose/models/visitor/PropertyDrivenVisitor.h"
namespace storm {
namespace modelchecker {

template <typename ValueType>
PropertyDrivenOpenMdpChecker<ValueType>::PropertyDrivenOpenMdpChecker(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager, storm::compose::benchmark::BenchmarkStats<ValueType>& stats)
    : AbstractOpenMdpChecker<ValueType>(manager, stats) {

}

template <typename ValueType>
void PropertyDrivenOpenMdpChecker<ValueType>::initializeParetoCurves() {
    // for each open MDP reachable from the root, create pareto curve that is
    // the hyper rectangle 0<=x<=1 for all dimensions x.
}

template <typename ValueType>
ApproximateReachabilityResult<ValueType> PropertyDrivenOpenMdpChecker<ValueType>::check(OpenMdpReachabilityTask task) {
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
    // do {
    //     auto rootPareto = paretoCurves.at(this->manager->getRoot()->getName());

    //     // find where to optimize. Ideally, all pareto curves are initialized to the complete probabilistic simplex.
    //     // we traverse the string diagram until we find some candidate which seems applicable for refinement.
    //     //
    //     // Any refinement step will ultimately always go down to the leaf nodes,
    //     // as the only way to refine sequence, sum, trace is by refining their
    //     // children.
    //     //
    //     // In other words, we could simply call some refine function on the root node repeatedly.
    //     // (Although it may be more efficient to call refine multiple times on other nodes instead)
    //     //
    //     // 

    // //} while(distance between rootPareto.lower and rootPareto.upper is smaller than epsilon);
    // } while(1);
}


template <typename ValueType>
std::vector<ValueType> PropertyDrivenOpenMdpChecker<ValueType>::getTargetWeight(OpenMdpReachabilityTask const& task) const {

}

template class PropertyDrivenOpenMdpChecker<storm::RationalNumber>;
template class PropertyDrivenOpenMdpChecker<double>;

}
}