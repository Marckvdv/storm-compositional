#pragma once

#include "storm-compose/models/ConcreteMdp.h"
#include "storm/modelchecker/multiobjective/Objective.h"

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
    std::shared_ptr<ConcreteMdp<ValueType>> toConcreteMdp(std::shared_ptr<OpenMdpManager<ValueType>> manager);
    ValueType getLowerBound(size_t entrance, bool leftEntrance, size_t exit, bool leftExit);
    bool isEmpty();
    bool hasEntrance(size_t entrance, bool leftEntrance);
    size_t getPointDimension() const;
    size_t getLeftEntrances() const;
    size_t getRightEntrances() const;

    std::vector<storm::modelchecker::multiobjective::Objective<ValueType>> getReachabilityObjectives();

   private:
    size_t getIndex(size_t entrance, bool leftEntrance) const;

    template<typename T>
    friend std::ostream& operator<<(std::ostream& os, BidirectionalReachabilityResult<T> const& result);

    size_t lEntrances, rEntrances, lExits, rExits;
    std::vector<std::vector<point>> points;
};

}  // namespace visitor
}  // namespace models
}  // namespace storm
