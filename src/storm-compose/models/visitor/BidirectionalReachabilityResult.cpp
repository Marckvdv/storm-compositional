#include "BidirectionalReachabilityResult.h"

namespace storm {
namespace models {
namespace visitor {

using storm::models::OpenMdp;
using storm::storage::SparseMatrixBuilder;
using storm::storage::SparseMatrix;
using storm::models::sparse::Mdp;

template<typename ValueType>
BidirectionalReachabilityResult<ValueType>::BidirectionalReachabilityResult(size_t lEntrances, size_t rEntrances, size_t lExits, size_t rExits) : lEntrances(lEntrances), rEntrances(rEntrances), lExits(lExits), rExits(rExits), points(lEntrances+rEntrances) {
}

template<typename ValueType>
size_t BidirectionalReachabilityResult<ValueType>::getIndex(size_t entrance, bool leftEntrance) const {
    if (leftEntrance) {
        return entrance;
    } else {
        return lEntrances + entrance;
    }
}

template<typename ValueType>
void BidirectionalReachabilityResult<ValueType>::addPoint(size_t entrance, bool leftEntrance, point paretoOptimalPoint) {
    size_t index = getIndex(entrance, leftEntrance);
    points[index].push_back(paretoOptimalPoint);
}

template<typename ValueType>
const std::vector<typename BidirectionalReachabilityResult<ValueType>::point>& BidirectionalReachabilityResult<ValueType>::getPoints(size_t entrance, bool leftEntrance) const {
    return points[getIndex(entrance, leftEntrance)];
}

template<typename ValueType>
std::shared_ptr<ConcreteMdp<ValueType>> BidirectionalReachabilityResult<ValueType>::toConcreteMdp()  {
    // Constructing a concrete MDP from a ParetoReachabilityResult is simply turning each point into an action,
    // e.g. the second point for entrance 0 is the second action for state 0, reaching the exits with the probabilities described by the point.
    storm::storage::SparseMatrixBuilder<ValueType> builder(0, 0, 0, true, true);

    // Order of the states is as follows:
    // [0, lEntrances-1] left entrances
    // [+1, +rEntrances - 1] right entrances
    // [+1, +lExits - 1] left exits
    // [+1, +rExits - 1] right exits

    const size_t
        lEntrancesStart = 0,
        rEntrancesStart = lEntrancesStart + lEntrances,
        lExitsStart = rEntrancesStart + rEntrances,
        rExitsStart = lExitsStart + lExits,
        entrances = lEntrances + rEntrances;

    size_t currentRow = 0;
    auto buildTransitions = [&](bool leftEntrance) {
        size_t entranceCount = leftEntrance ? lEntrances : rEntrances;

        for (size_t entrance = 0; entrance < entranceCount; ++entrance) {
            builder.newRowGroup(currentRow);
            const auto& points = getPoints(entrance, leftEntrance);
            for (const auto& point : points) {
                STORM_LOG_ASSERT(point.size() == lExits + rExits, "expected point size to line up with amount of exits");
                for (size_t k = 0; k < point.size(); ++k) {
                    builder.addNextValue(currentRow, entrances + k, point[k]);
                }
                ++currentRow;
            }
        }
    //    for (size_t entrance = 0; entrance < entrances; ++entrance) {
    //        builder.newRowGroup(currentRow);

    //        const auto& points = getPoints(entrance);
    //        for (size_t j = 0; j < points.size(); ++j) {
    //            const auto& point = points[j];
    //            for (size_t k = 0; k < point.size(); ++k) {
    //                builder.addNextValue(currentRow, entrances + k, point[k]);
    //            }
    //            ++currentRow;
    //        }
    //    }
    };

    buildTransitions(true); // left entrances
    buildTransitions(false); // right entrances

    const size_t totalStateCount = lEntrances + rEntrances + lExits + rExits;
    for (size_t i = builder.getCurrentRowGroupCount(); i < totalStateCount; ++i) {
        builder.newRowGroup(currentRow);
        builder.addNextValue(currentRow, i, 1);
        ++currentRow;
    }

    std::vector<size_t> lEntrance, rEntrance, lExit, rExit;
    size_t currentState = 0;
    auto pushEntrancesExits = [&] (auto &entranceExit, const size_t count) {
        for (size_t i = 0; i < count; ++i, ++currentState) {
            entranceExit.push_back(currentState);
        }
    };
    pushEntrancesExits(lEntrance, lEntrances);
    pushEntrancesExits(rEntrance, rEntrances);
    pushEntrancesExits(lExit, lExits);
    pushEntrancesExits(rExit, rExits);

    /*
    auto matrix = builder.build();
    storm::models::sparse::StateLabeling labeling(matrix.getRowGroupCount());
    auto newMdp = std::make_shared<Mdp<ValueType>>(matrix, labeling);
    this->current = ConcreteMdp<ValueType>(this->manager, newMdp, lEntrance, rEntrance, lExit, rExit);
    */
}

template class BidirectionalReachabilityResult<double>;
template class BidirectionalReachabilityResult<storm::RationalNumber>;

}
}
}
