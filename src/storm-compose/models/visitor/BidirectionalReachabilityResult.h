#pragma once

#include "storm-compose/models/ConcreteMdp.h"

#include <vector>

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
class BidirectionalReachabilityResult {
public:
    typedef std::vector<ValueType> point;

    BidirectionalReachabilityResult() = default;
    BidirectionalReachabilityResult(size_t lEntrances, size_t rEntrances, size_t lExits, size_t rExits);
    void addPoint(size_t entrance, bool leftEntrance, point paretoOptimalPoint);
    const std::vector<point>& getPoints(size_t entrance, bool leftEntrance) const;
    std::shared_ptr<ConcreteMdp<ValueType>> toConcreteMdp();

    // TODO later
    //void setReward(size_t entrance, size_t exit, ValueType value);
    //ValueType getReward(size_t entrance, size_t exit);

private:
    size_t getIndex(size_t entrance, bool leftEntrance) const;

    size_t lEntrances, rEntrances, lExits, rExits;
    std::vector<std::vector<point>> points;
};

}
}
}
