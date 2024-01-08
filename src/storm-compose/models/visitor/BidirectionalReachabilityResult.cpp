#include "BidirectionalReachabilityResult.h"

#include "storm-parsers/parser/FormulaParser.h"

namespace storm {
namespace models {
namespace visitor {

using storm::models::OpenMdp;
using storm::models::sparse::Mdp;
using storm::storage::SparseMatrix;
using storm::storage::SparseMatrixBuilder;

template<typename ValueType>
BidirectionalReachabilityResult<ValueType>::BidirectionalReachabilityResult(size_t lEntrances, size_t rEntrances, size_t lExits, size_t rExits)
    : lEntrances(lEntrances), rEntrances(rEntrances), lExits(lExits), rExits(rExits), points(lEntrances + rEntrances) {}

template<typename ValueType>
BidirectionalReachabilityResult<ValueType>::BidirectionalReachabilityResult(ConcreteMdp<ValueType> const& mdp)
    : BidirectionalReachabilityResult<ValueType>(mdp.getLEntranceCount(), mdp.getREntranceCount(), mdp.getLExitCount(), mdp.getRExitCount()) {}

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
    STORM_LOG_ASSERT(index < points.size(), "sanity check" << index << " vs " << points.size());
    // ValueType sum = 0;
    for (auto& v : paretoOptimalPoint) {
        // STORM_LOG_THROW(v > -1e-8, storm::exceptions::InvalidOperationException, "negative point value: " << v);
        if (v < 0)
            v = 0;
        // STORM_LOG_THROW(v < 1.0001, storm::exceptions::InvalidOperationException, "point greather than 1: " << v);
        // sum += v;
    }
    // STORM_LOG_THROW(sum > 0.999 && sum < 1.0001, storm::exceptions::InvalidOperationException, "sum not 1: " << sum);

    points[index].push_back(paretoOptimalPoint);
}

template<typename ValueType>
const std::vector<typename BidirectionalReachabilityResult<ValueType>::point>& BidirectionalReachabilityResult<ValueType>::getPoints(size_t entrance,
                                                                                                                                     bool leftEntrance) const {
    return points[getIndex(entrance, leftEntrance)];
}

template<typename ValueType>
std::shared_ptr<ConcreteMdp<ValueType>> BidirectionalReachabilityResult<ValueType>::toConcreteMdp(std::shared_ptr<OpenMdpManager<ValueType>> manager) {
    // Constructing a concrete MDP from a ParetoReachabilityResult is simply turning each point into an action,
    // e.g. the second point for entrance 0 is the second action for state 0, reaching the exits with the probabilities described by the point.

    // Order of the states is as follows:
    // [0, lEntrances - 1] left entrances
    // [+1, +rEntrances - 1] right entrances
    // [+1, +lExits - 1] left exits
    // [+1, +rExits - 1] right exits

    const size_t lEntrancesStart = 0, rEntrancesStart = lEntrancesStart + lEntrances, lExitsStart = rEntrancesStart + rEntrances,
                 rExitsStart = lExitsStart + lExits, entrances = lEntrances + rEntrances, totalStateCount = lEntrances + rEntrances + lExits + rExits + 1,
                 sinkState = totalStateCount - 1;

    storm::storage::SparseMatrixBuilder<ValueType> builder(0, 0, 0, true, true);
    storm::models::sparse::StateLabeling labeling(totalStateCount);
    labeling.addLabel("init");

    size_t currentRow = 0, currentState = 0;
    auto buildTransitions = [&](bool leftEntrance) {
        size_t entranceCount = leftEntrance ? lEntrances : rEntrances;

        for (size_t entrance = 0; entrance < entranceCount; ++entrance) {
            builder.newRowGroup(currentRow);
            size_t index = getIndex(entrance, leftEntrance);

            std::string label = (leftEntrance ? "len" : "ren") + std::to_string(entrance);
            labeling.addLabel(label);
            labeling.addLabelToState(label, currentState);

            // Keep track of total probability mass
            const auto& points = getPoints(entrance, leftEntrance);
            // std::cout << "size of points: " << points.size() << std::endl;
            // std::cout << (leftEntrance ? "left " : "right ") << "entrance " << entrance << std::endl;
            // if (points.empty()) {
            //     std::cout << "WARN: Points was empty (?)" << std::endl;
            //     throw 1;
            // }

            for (const auto& point : points) {
                ValueType probabilitySum = storm::utility::zero<ValueType>();
                STORM_LOG_ASSERT(point.size() == lExits + rExits, "expected point size to line up with amount of exits");
                for (size_t k = 0; k < point.size(); ++k) {
                    builder.addNextValue(currentRow, entrances + k, point[k]);
                    probabilitySum += point[k];
                }

                ValueType remainingProbability = storm::utility::one<ValueType>() - probabilitySum;
                if (remainingProbability > storm::utility::zero<ValueType>()) {
                    builder.addNextValue(currentRow, sinkState, remainingProbability);
                }
                ++currentRow;
            }
            ++currentState;
        }
    };

    buildTransitions(true);   // left entrances
    buildTransitions(false);  // right entrances

    // Turn exits and sink state into self-loop states
    for (size_t i = builder.getCurrentRowGroupCount(); i < totalStateCount; ++i) {
        builder.newRowGroup(currentRow);
        builder.addNextValue(currentRow, i, 1);
        ++currentRow;
    }

    std::vector<size_t> lEntrance, rEntrance, lExit, rExit;
    currentState = 0;
    auto pushEntrancesExits = [&](auto& entranceExit, const size_t count, bool exit, bool leftExit) {
        for (size_t i = 0; i < count; ++i, ++currentState) {
            entranceExit.push_back(currentState);
            if (exit) {
                std::string label = (leftExit ? "lex" : "rex") + std::to_string(i);
                labeling.addLabel(label);
                labeling.addLabelToState(label, currentState);
            }
        }
    };
    pushEntrancesExits(lEntrance, lEntrances, false, false);
    pushEntrancesExits(rEntrance, rEntrances, false, false);
    pushEntrancesExits(lExit, lExits, true, true);
    pushEntrancesExits(rExit, rExits, true, false);

    auto matrix = builder.build();
    auto newMdp = std::make_shared<Mdp<ValueType>>(matrix, labeling);
    return std::make_shared<ConcreteMdp<ValueType>>(manager, newMdp, lEntrance, rEntrance, lExit, rExit);
}

template<typename ValueType>
ValueType BidirectionalReachabilityResult<ValueType>::getLowerBound(size_t entrance, bool leftEntrance, size_t exit, bool leftExit) {
    const auto& points = getPoints(entrance, leftEntrance);
    const size_t exitIndex = exit + (leftExit ? 0 : lExits);

    ValueType maxReach = storm::utility::zero<ValueType>();
    // Find point that maximizes our target
    for (const auto& point : points) {
        ValueType reachProb = point[exitIndex];
        if (reachProb > maxReach) {
            maxReach = reachProb;
        }
    }

    return maxReach;
}

template<typename ValueType>
bool BidirectionalReachabilityResult<ValueType>::isEmpty() {
    return points.empty();
}

template<typename ValueType>
bool BidirectionalReachabilityResult<ValueType>::hasEntrance(size_t entrance, bool leftEntrance) {
    if (leftEntrance) {
        return entrance < lEntrances;
    } else {
        return entrance < rEntrances;
    }
}

template<typename ValueType>
std::ostream& operator<<(std::ostream& os, BidirectionalReachabilityResult<ValueType> const& result) {
    os << "Bidirectional reachability result:" << std::endl;

    auto printPoint = [&](const auto& point) {
        ValueType sum = 0;
        for (size_t lExit = 0; lExit < result.lExits; ++lExit) {
            auto value = point[lExit];
            if (value != 0) {
                os << "lExit " << lExit << ": " << value << std::endl;
            }
            sum += value;
        }
        os << "Sum: " << sum << std::endl;

        sum = 0;
        for (size_t rExit = 0; rExit < result.rExits; ++rExit) {
            auto value = point[result.lExits + rExit];
            if (value != 0) {
                os << "rExit " << rExit << ": " << value << std::endl;
            }
            sum += value;
        }
        os << "Sum: " << sum << std::endl;
    };

    for (size_t lEntrance = 0; lEntrance < result.lEntrances; ++lEntrance) {
        os << "lEntrance " << lEntrance << ":" << std::endl;
        size_t idx = 0;
        for (const auto& point : result.getPoints(lEntrance, true)) {
            os << "point " << idx << ":" << std::endl;
            printPoint(point);
            os << std::endl;
            ++idx;
        }
        os << std::endl;
    }
    for (size_t rEntrance = 0; rEntrance < result.rEntrances; ++rEntrance) {
        os << "rEntrance " << rEntrance << ":" << std::endl;
        size_t idx = 0;
        for (const auto& point : result.getPoints(rEntrance, false)) {
            os << "point " << idx;
            printPoint(point);
            os << std::endl;
            ++idx;
        }
        os << std::endl;
    }
    return os;
}

template<typename ValueType>
size_t BidirectionalReachabilityResult<ValueType>::getPointDimension() const {
    return lExits + rExits;
}

template<typename ValueType>
size_t BidirectionalReachabilityResult<ValueType>::getLeftEntrances() const {
    return lEntrances;
}

template<typename ValueType>
size_t BidirectionalReachabilityResult<ValueType>::getRightEntrances() const {
    return rEntrances;
}

template<typename ValueType>
std::vector<storm::modelchecker::multiobjective::Objective<ValueType>> BidirectionalReachabilityResult<ValueType>::getReachabilityObjectives() {
    std::vector<storm::modelchecker::multiobjective::Objective<ValueType>> objectives;

    auto pushExitObjectives = [&](bool leftExit) {
        size_t exits = leftExit ? lExits : rExits;
        for (size_t i = 0; i < exits; ++i) {
            std::stringstream formulaBuffer;

            std::string exitMarker = leftExit ? "lex" : "rex";
            formulaBuffer << "Pmax=? [F \"" << exitMarker << i << "\"]";

            storm::parser::FormulaParser formulaParser;
            auto formula = formulaParser.parseSingleFormulaFromString(formulaBuffer.str());

            storm::modelchecker::multiobjective::Objective<ValueType> objective;
            objective.originalFormula = formula;
            objectives.push_back(objective);
        }
    };
    pushExitObjectives(true);
    pushExitObjectives(false);

    return objectives;
}

template class BidirectionalReachabilityResult<double>;
template class BidirectionalReachabilityResult<storm::RationalNumber>;

template std::ostream& operator<<(std::ostream& os, BidirectionalReachabilityResult<double> const& result);
template std::ostream& operator<<(std::ostream& os, BidirectionalReachabilityResult<storm::RationalNumber> const& result);

}  // namespace visitor
}  // namespace models
}  // namespace storm
