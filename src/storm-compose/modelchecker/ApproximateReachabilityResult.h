#pragma once

#include <iostream>
#include "utility/constants.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
class ApproximateReachabilityResult {
   public:
    ApproximateReachabilityResult();
    ApproximateReachabilityResult(ValueType lower, ValueType upper);
    ApproximateReachabilityResult(ValueType exactValue);

    ValueType getLowerBound() const;
    ValueType getUpperBound() const;
    ValueType getError() const;
    bool isExact(ValueType epsilon) const;
    bool isExact() const;
    static ApproximateReachabilityResult<ValueType> combineLowerUpper(ApproximateReachabilityResult<ValueType> const& lower,
                                                                      ApproximateReachabilityResult<ValueType> const& upper);

   private:
    template<typename T>
    friend std::ostream& operator<<(std::ostream& os, ApproximateReachabilityResult<T> const& result);

    bool soundUpperBound;
    ValueType lower, upper;
};

template<typename ValueType>
std::ostream& operator<<(std::ostream& os, ApproximateReachabilityResult<ValueType> const& result) {
    // if (result.soundUpperBound) {
    //     os << "<" << result.lower << ", " << result.upper << ">" << std::endl;
    // } else {
    //     os << "<" << result.lower << ", ?>" << std::endl;
    // }

    if (result.soundUpperBound) {
        os << "<" << storm::utility::convertNumber<double>(result.lower) << ", " << storm::utility::convertNumber<double>(result.upper) << ">" << std::endl;
    } else {
        os << "<" << storm::utility::convertNumber<double>(result.lower) << ", ?>" << std::endl;
    }
    return os;
}

}  // namespace modelchecker
}  // namespace storm
