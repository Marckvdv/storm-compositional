#pragma once

#include "AbstractOpenMdpChecker.h"

namespace storm {
namespace modelchecker {

template <typename ValueType>
class WeightedOpenMdpChecker : public AbstractOpenMdpChecker<ValueType> {

};

}
}