#pragma once

#include "utility/constants.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
class ApproximateReachabilityResult {
   public:
    ApproximateReachabilityResult() : lower(0), upper(0) {}

    ApproximateReachabilityResult(ValueType lower, ValueType upper) : lower(lower), upper(upper) {
        STORM_LOG_ASSERT(lower <= upper, "Lower bound was higher than the upper bound");
    }

    ApproximateReachabilityResult(ValueType exactValue) : lower(exactValue), upper(exactValue) {}

    ValueType getLowerBound() {
        return lower;
    }

    ValueType getUpperBound() {
        return upper;
    }

    bool isExact(ValueType epsilon) {
        return (upper - lower) < epsilon;
    }

    bool isExact() {
        return lower == upper;
    }

   private:
    template<typename T>
    friend std::ostream& operator<<(std::ostream& os, ApproximateReachabilityResult<T> const& result);

    ValueType lower, upper;
};

template<typename ValueType>
std::ostream& operator<<(std::ostream& os, ApproximateReachabilityResult<ValueType> const& result) {
    os << "<" << storm::utility::convertNumber<double>(result.lower) << ", " << storm::utility::convertNumber<double>(result.upper) << ">" << std::endl;
    return os;
}

}  // namespace modelchecker
}  // namespace storm
