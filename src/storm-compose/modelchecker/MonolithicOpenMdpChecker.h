#pragma once

#include "AbstractOpenMdpChecker.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
class MonolithicOpenMdpChecker : public AbstractOpenMdpChecker<ValueType> {
   public:
    MonolithicOpenMdpChecker(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager, storm::compose::benchmark::BenchmarkStats<ValueType>& stats);
    ApproximateReachabilityResult<ValueType> check(OpenMdpReachabilityTask task) override;

    ApproximateReachabilityResult<ValueType> checkConcreteMdp(storm::models::ConcreteMdp<ValueType> const& concreteMdp, OpenMdpReachabilityTask task);
};

}  // namespace modelchecker
}  // namespace storm
