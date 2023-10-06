#pragma once

#include "AbstractOpenMdpChecker.h"

//#include "storm/storage/geometry/NativePolytope.h"

namespace storm {
namespace modelchecker {

template <typename ValueType>
struct ParetoCurve {
//    storm::storage::geometry::NativePolytope<ValueType> lower, upper;
};

template <typename ValueType>
class PropertyDrivenOpenMdpChecker : public AbstractOpenMdpChecker<ValueType> {
public:
    PropertyDrivenOpenMdpChecker(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager, storm::compose::benchmark::BenchmarkStats<ValueType>& stats);

    virtual ApproximateReachabilityResult<ValueType> check(OpenMdpReachabilityTask task) override;

private:
    void initializeParetoCurves();

    std::vector<ValueType> getTargetWeight(OpenMdpReachabilityTask const& task) const;

    // Pareto curve storage
    // A pareto curve is a pair of polytopes representing an lower and upper bound.
    // A polytope is the intersection between its stored halfspaces, which are stored by their normals and offset.
    std::map<std::string, ParetoCurve<ValueType>> paretoCurves;
};

}
}