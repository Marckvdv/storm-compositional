#include "ApproximateReachabilityResult.h"

#include "storm/utility/macros.h"
#include "storm/adapters/RationalNumberAdapter.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
ApproximateReachabilityResult<ValueType>::ApproximateReachabilityResult() : lower(0), upper(0) {}

template<typename ValueType>
ApproximateReachabilityResult<ValueType>::ApproximateReachabilityResult(ValueType lower, ValueType upper) : lower(lower), upper(upper), soundUpperBound(true) {
    STORM_LOG_ASSERT(lower - 1e-6 <= upper, "Lower bound was higher than the upper bound");
}

template<typename ValueType>
ApproximateReachabilityResult<ValueType>::ApproximateReachabilityResult(ValueType exactValue) : lower(exactValue), upper(exactValue), soundUpperBound(false) {}

template<typename ValueType>
ValueType ApproximateReachabilityResult<ValueType>::getLowerBound() const {
    return lower;
}

template<typename ValueType>
ValueType ApproximateReachabilityResult<ValueType>::getUpperBound() const {
    return upper;
}

template<typename ValueType>
ValueType ApproximateReachabilityResult<ValueType>::getError() const {
    return upper - lower;
}

template<typename ValueType>
bool ApproximateReachabilityResult<ValueType>::isExact(ValueType epsilon) const {
    return (upper - lower) < epsilon;
}

template<typename ValueType>
bool ApproximateReachabilityResult<ValueType>::isExact() const {
    return lower == upper;
}

template<typename ValueType>
ApproximateReachabilityResult<ValueType> ApproximateReachabilityResult<ValueType>::combineLowerUpper(ApproximateReachabilityResult<ValueType> const& lower, ApproximateReachabilityResult<ValueType> const& upper) {
    return ApproximateReachabilityResult<ValueType>(lower.lower, upper.upper);
}

template class ApproximateReachabilityResult<double>;
template class ApproximateReachabilityResult<storm::RationalNumber>;

}  // namespace modelchecker
}  // namespace storm
