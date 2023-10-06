#include "PropertyDrivenOpenMdpChecker.h"

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
    do {
        auto rootPareto = paretoCurves.at(this->manager->getRoot()->getName());

        // find where to optimize. Ideally, all pareto curves are initialized to the complete probabilistic simplex.
        // we traverse the string diagram until we find some candidate which seems applicable for refinement.
        //
        // Any refinement step will ultimately always go down to the leaf nodes,
        // as the only way to refine sequence, sum, trace is by refining their
        // children.
        //
        // In other words, we could simply call some refine function on the root node repeatedly.
        // (Although it may be more efficient to call refine multiple times on other nodes instead)
        //
        // 

    //} while(distance between rootPareto.lower and rootPareto.upper is smaller than epsilon);
    } while(1);
}


template <typename ValueType>
std::vector<ValueType> PropertyDrivenOpenMdpChecker<ValueType>::getTargetWeight(OpenMdpReachabilityTask const& task) const {

}

template class PropertyDrivenOpenMdpChecker<storm::RationalNumber>;
template class PropertyDrivenOpenMdpChecker<double>;

}
}