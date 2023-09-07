#pragma once

#include "AbstractOpenMdpChecker.h"

namespace storm {
namespace modelchecker {

template <typename ValueType>
class MonolithicOpenMdpChecker : public AbstractOpenMdpChecker<ValueType> {
public:
    MonolithicOpenMdpChecker(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager);
    ApproximateReachabilityResult<ValueType> check(OpenMdpReachabilityTask task) override;

private:
};

}
}