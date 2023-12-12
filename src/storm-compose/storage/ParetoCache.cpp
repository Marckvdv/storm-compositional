#include "ParetoCache.h"
#include "exceptions/InvalidArgumentException.h"
#include "modelchecker/prctl/SparseDtmcPrctlModelChecker.h"
#include "storage/geometry/Halfspace.h"
#include "storage/geometry/Polytope.h"
#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/storage/EntranceExit.h"
#include "storm/environment/Environment.h"
#include "storm/modelchecker/prctl/helper/SparseDtmcPrctlHelper.h"
#include "storm/solver/SolveGoal.h"
#include "storm/utility/constants.h"

namespace storm {
namespace storage {

template<typename ValueType>
boost::optional<std::vector<ValueType>> ParetoCache<ValueType>::getLowerBound(models::ConcreteMdp<ValueType>* ptr, std::vector<ValueType> outputWeight) {
    if (!isInitialized(ptr)) {
        initializeParetoCurve(ptr);
    }

    std::vector<ValueType> lowerBound(ptr->getEntranceCount());

    size_t weightIndex = 0;
    auto processEntrances = [&](const auto& entrances, storage::EntranceExit entrance) {
        for (size_t i = 0; i < entrances.size(); ++i) {
            Position pos = {entrance, i};
            auto paretoEntry = getLowerUpper(ptr, outputWeight, pos);
            if (!paretoEntry)
                return false;

            const auto& lb = paretoEntry->first;
            const auto& ub = paretoEntry->second;

            ValueType error = getError(lb, ub);
            if (error > this->errorTolerance)
                return false;
            lowerBound[weightIndex] = innerProduct(lb, outputWeight);

            ++weightIndex;
        }
        return true;
    };

    bool result1 = processEntrances(ptr->getLEntrance(), storage::L_ENTRANCE);
    if (!result1)
        return boost::none;

    bool result2 = processEntrances(ptr->getREntrance(), storage::R_ENTRANCE);
    if (!result2)
        return boost::none;

    return lowerBound;
}

template<typename ValueType>
void ParetoCache<ValueType>::addToCache(models::ConcreteMdp<ValueType>* ptr, std::vector<ValueType> outputWeight, std::vector<ValueType> inputWeight,
                                        boost::optional<storm::storage::Scheduler<ValueType>> sched) {
    STORM_LOG_THROW(sched, storm::exceptions::InvalidArgumentException, "need scheduler present");

    if (!isInitialized(ptr)) {
        initializeParetoCurve(ptr);
    }

    size_t stateCount = ptr->getMdp()->getTransitionMatrix().getRowGroupCount();
    storage::BitVector phi(stateCount, true);
    storm::Environment env;

    // Store point for each entrance
    std::vector<std::vector<ValueType>> points(ptr->getEntranceCount());

    // Compute induced Markov chain
    auto mc = ptr->getMdp()->applyScheduler(*sched, false);

    // For each exit, compute the reachability probabilty for each entrance
    size_t weightIndex = 0;
    auto processExits = [&](const auto& exits) {
        for (size_t exitState : exits) {
            storage::BitVector psi(stateCount);
            psi.set(exitState);

            storm::solver::SolveGoal<ValueType> goal(false);
            modelchecker::helper::SparseDtmcPrctlHelper<ValueType> helper;
            auto values = helper.computeUntilProbabilities(env, std::move(goal), mc->getTransitionMatrix(), mc->getBackwardTransitions(), phi, psi, false);

            size_t entranceIndex = 0;
            for (size_t entranceState : ptr->getLEntrance()) {
                points[entranceIndex].push_back(values[entranceState]);
                ++entranceIndex;
            }
            for (size_t entranceState : ptr->getREntrance()) {
                points[entranceIndex].push_back(values[entranceState]);
                ++entranceIndex;
            }

            ++weightIndex;
        }
    };
    processExits(ptr->getLExit());
    processExits(ptr->getRExit());

    // Now that we have the points, update the lower and upper bounds:
    size_t entranceIndex = 0;
    size_t leftEntranceCount = ptr->getLEntrance().size();
    for (const auto& p : points) {
        bool leftEntrance = entranceIndex < leftEntranceCount;
        storage::EntranceExit entranceExit = leftEntrance ? storage::L_ENTRANCE : storage::R_ENTRANCE;
        size_t realEntranceIndex = leftEntrance ? entranceIndex : entranceIndex - leftEntranceCount;
        Position pos{entranceExit, realEntranceIndex};

        std::pair<models::ConcreteMdp<ValueType>*, Position> key{ptr, pos};

        updateLowerUpperBounds(key, p, outputWeight);

        ++entranceIndex;
    }
}

template<typename ValueType>
void ParetoCache<ValueType>::updateLowerUpperBounds(std::pair<models::ConcreteMdp<ValueType>*, Position> key, std::vector<ValueType> point,
                                                    std::vector<ValueType> weight) {
    auto& lb = lowerBounds[key];
    auto& ub = upperBounds[key];

    auto newPointPolytope = storage::geometry::Polytope<ValueType>::create({point});
    storage::geometry::Halfspace<ValueType> newHalfspace(weight, point);

    lb = lb->convexUnion(newPointPolytope);
    ub = ub->intersection(newHalfspace);
}

template<typename ValueType>
bool ParetoCache<ValueType>::isInitialized(models::ConcreteMdp<ValueType>* ptr) {
    Position leftPos{L_ENTRANCE, 0};
    const auto it1 = lowerBounds.find({ptr, leftPos});
    if (it1 != lowerBounds.end()) {
        return true;
    }

    Position rightPos{R_ENTRANCE, 0};
    const auto it2 = lowerBounds.find({ptr, rightPos});
    if (it2 != lowerBounds.end()) {
        return true;
    }

    return false;
}

template<typename ValueType>
void ParetoCache<ValueType>::initializeParetoCurve(models::ConcreteMdp<ValueType>* ptr) {
    // TODO FIXME
    for (size_t i = 0; i < ptr->getLEntrance().size(); ++i) {
        Position pos{L_ENTRANCE, i};
        std::pair<models::ConcreteMdp<ValueType>*, Position> key{ptr, pos};

        lowerBounds[key] = geometry::Polytope<ValueType>::createEmptyPolytope();
        upperBounds[key] = geometry::Polytope<ValueType>::createUniversalPolytope();
    }

    for (size_t i = 0; i < ptr->getREntrance().size(); ++i) {
        Position pos{R_ENTRANCE, i};
        std::pair<models::ConcreteMdp<ValueType>*, Position> key{ptr, pos};

        lowerBounds[key] = geometry::Polytope<ValueType>::createEmptyPolytope();
        upperBounds[key] = geometry::Polytope<ValueType>::createUniversalPolytope();
    }
}

template<typename ValueType>
bool ParetoCache<ValueType>::needScheduler() {
    return true;
}

template<typename ValueType>
boost::optional<std::pair<std::vector<ValueType>, std::vector<ValueType>>> ParetoCache<ValueType>::getLowerUpper(models::ConcreteMdp<ValueType>* ptr,
                                                                                                                 WeightType outputWeight, Position pos) {
    std::pair<models::ConcreteMdp<ValueType>*, Position> key = {ptr, pos};

    auto lowerBoundPolytope = lowerBounds[key];
    auto upperBoundPolytope = upperBounds[key];

    size_t dim = outputWeight.size();
    std::vector<ValueType> negativeWeight(dim);
    for (size_t i = 0; i < dim; ++i) {
        negativeWeight[i] = -outputWeight[i];
    }

    auto lb = lowerBoundPolytope->optimize(outputWeight);
    auto ub = upperBoundPolytope->optimize(negativeWeight);

    if (!ub.second || !lb.second)
        return boost::none;

    return std::make_pair(lb.first, ub.first);
}

template<typename ValueType>
ValueType ParetoCache<ValueType>::innerProduct(WeightType a, WeightType b) {
    STORM_LOG_ASSERT(a.size() == b.size(), "vectors must have the same size");

    ValueType result = storm::utility::zero<ValueType>();
    for (size_t i = 0; i < a.size(); ++i) {
        result += a[i] * b[i];
    }
    return result;
}

template<typename ValueType>
ValueType ParetoCache<ValueType>::getError(WeightType lower, WeightType upper) {
    STORM_LOG_ASSERT(lower.size() == upper.size(), "size mismatch");

    ValueType maxError = -storm::utility::infinity<ValueType>();
    for (size_t i = 0; i < upper.size(); ++i) {
        ValueType error = upper[i] - lower[i];
        if (error > maxError) {
            maxError = error;
        }
    }

    return maxError;
}

template class ParetoCache<storm::RationalNumber>;
template class ParetoCache<double>;

}  // namespace storage
}  // namespace storm
