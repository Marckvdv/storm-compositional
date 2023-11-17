#pragma once

#include "AbstractOpenMdpChecker.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
class WeightedOpenMdpChecker : public AbstractOpenMdpChecker<ValueType> {
   public:
    WeightedOpenMdpChecker(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager, storm::compose::benchmark::BenchmarkStats<ValueType>& stats);
    ApproximateReachabilityResult<ValueType> check(OpenMdpReachabilityTask task) override;

   private:
};

}  // namespace modelchecker
}  // namespace storm