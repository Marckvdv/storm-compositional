#pragma once

#include "AbstractOpenMdpChecker.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
class MonolithicOpenMdpChecker : public AbstractOpenMdpChecker<ValueType> {
   public:
    MonolithicOpenMdpChecker(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager, storm::compose::benchmark::BenchmarkStats<ValueType>& stats);
    ApproximateReachabilityResult<ValueType> check(OpenMdpReachabilityTask task) override;
};

}  // namespace modelchecker
}  // namespace storm