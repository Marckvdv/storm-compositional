#pragma once

#include "AbstractOpenMdpChecker.h"

namespace storm {
namespace modelchecker {

template <typename ValueType>
class NaiveOpenMdpChecker2 : public AbstractOpenMdpChecker<ValueType> {
public:
    NaiveOpenMdpChecker2(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager);
    ApproximateReachabilityResult<ValueType> check(OpenMdpReachabilityTask task) override;

private:
};

}
}