#include "ParetoCache.h"
#include "environment/solver/MinMaxSolverEnvironment.h"
#include "environment/solver/SolverEnvironment.h"
#include "exceptions/BaseException.h"
#include "exceptions/InvalidArgumentException.h"
#include "modelchecker/prctl/SparseDtmcPrctlModelChecker.h"
#include "solver/SolverSelectionOptions.h"
#include "storage/geometry/Halfspace.h"
#include "storage/geometry/Polytope.h"
#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/models/visitor/BidirectionalReachabilityResult.h"
#include "storm-compose/storage/EntranceExit.h"
#include "storm/environment/Environment.h"
#include "storm/modelchecker/prctl/helper/SparseDtmcPrctlHelper.h"
#include "storm/solver/SolveGoal.h"
#include "storm/utility/constants.h"
#include "storm/utility/vector.h"
#include "utility/Stopwatch.h"

namespace storm {
namespace storage {

template<typename ValueType>
boost::optional<typename ParetoCache<ValueType>::WeightType> ParetoCache<ValueType>::getLowerBound(models::ConcreteMdp<ValueType>* ptr,
                                                                                                   WeightType outputWeight) {
    // std::cout << "Cache context, model " << ptr->getName() << " weight " << storm::utility::vector::toString(outputWeight) << std::endl;;

    ParetoPointType convertedOutputWeight = storm::utility::vector::convertNumericVector<ParetoRational>(outputWeight);
    WeightType lowerBound(ptr->getEntranceCount());

    size_t weightIndex = 0;
    auto processEntrances = [&](const auto& entrances, storage::EntranceExit entrance) {
        for (size_t i = 0; i < entrances.size(); ++i) {
            Position pos = {entrance, i};
            const auto& lb = getBestLowerBound(ptr, convertedOutputWeight, pos);
            lowerBound[weightIndex] = storm::utility::convertNumber<ValueType>(storm::utility::vector::dotProduct(lb, convertedOutputWeight));

            ++weightIndex;
        }
    };

    processEntrances(ptr->getLEntrance(), storage::L_ENTRANCE);
    processEntrances(ptr->getREntrance(), storage::R_ENTRANCE);

    return lowerBound;
}

// TODO reuse code above instead of copying
template<typename ValueType>
boost::optional<typename ParetoCache<ValueType>::WeightType> ParetoCache<ValueType>::getUpperBound(models::ConcreteMdp<ValueType>* ptr,
                                                                                                   WeightType outputWeight) {
    ParetoPointType convertedOutputWeight = storm::utility::vector::convertNumericVector<ParetoRational>(outputWeight);
    WeightType upperBound(ptr->getEntranceCount());

    size_t weightIndex = 0;
    auto processEntrances = [&](const auto& entrances, storage::EntranceExit entrance) {
        for (size_t i = 0; i < entrances.size(); ++i) {
            Position pos = {entrance, i};
            try {
                const auto& ub = getBestUpperBound(ptr, convertedOutputWeight, pos);
                upperBound[weightIndex] = storm::utility::convertNumber<ValueType>(storm::utility::vector::dotProduct(ub, convertedOutputWeight));
            } catch (storm::exceptions::BaseException e) {
                return false;
            }
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

    // std::cout << "Cache hit" << std::endl;

    return upperBound;
}

template<typename ValueType>
void ParetoCache<ValueType>::addToCache(models::ConcreteMdp<ValueType>* ptr, std::vector<ValueType> outputWeight, std::vector<ValueType> inputWeight,
                                        boost::optional<storm::storage::Scheduler<ValueType>> sched) {
    STORM_LOG_THROW(sched, storm::exceptions::InvalidArgumentException, "need scheduler present");
    ParetoPointType convertedOutputWeight = storm::utility::vector::convertNumericVector<ParetoRational>(outputWeight);
    ParetoPointType normalizedOutputWeight = ParetoPointType(convertedOutputWeight);
    storm::utility::vector::normalizeInPlace(normalizedOutputWeight);

    // std::cout << "Adding something to the cache of concrete mdp " << ptr->getName() << std::endl;

    size_t stateCount = ptr->getMdp()->getTransitionMatrix().getRowGroupCount();
    storage::BitVector phi(stateCount, true);

    storm::Environment env;

    // TODO make configurable:
    env.solver().minMax().setMethod(solver::MinMaxMethod::OptimisticValueIteration);
    env.solver().minMax().setPrecision(1e-5);
    // Store point for each entrance
    std::vector<ParetoPointType> points(ptr->getEntranceCount());

    // Compute induced Markov chain
    auto mc = ptr->getMdp()->applyScheduler(*sched, false);

    // For each exit, compute the reachability probabilty for each entrance
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
        updateLowerUpperBounds(key, p, normalizedOutputWeight);

        ++entranceIndex;
    }

    // auto lb = *getLowerBound(ptr, outputWeight);
    // auto ub = *getUpperBound(ptr, outputWeight);

    // std::cout << "CHECK: lb, ub, input" << std::endl;
    // std::cout << storm::utility::vector::toString<ValueType>(lb) << std::endl;
    // std::cout << storm::utility::vector::toString<ValueType>(ub) << std::endl;
    // std::cout << storm::utility::vector::toString<ValueType>(inputWeight) << std::endl;

    // getBestLowerBound(ptr, outputWeight, Position pos)
}

template<typename ValueType>
void ParetoCache<ValueType>::updateLowerUpperBounds(std::pair<models::ConcreteMdp<ValueType>*, Position> key, ParetoPointType point, ParetoPointType weight) {
    auto& lb = lowerBounds[key];
    auto& ub = upperBounds[key];

    storage::geometry::Halfspace<ParetoRational> newHalfspace(weight, point);  // TODO add OVI epsilon

    lb.push_back(point);
    ub = ub->intersection(newHalfspace);
}

template<typename ValueType>
bool ParetoCache<ValueType>::isInitialized(models::ConcreteMdp<ValueType>* ptr) const {
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
    // Initially, we know nothing about the MDP, so for every exit, the
    // reachability probability is atleast 0 and at most 1.
    //
    // This means that the lower bound is the polytope that only contains the
    // origin, while the upper bound contains the whole probabilistic simplex.
    size_t dimension = ptr->getExitCount();
    auto initializeEntrances = [&](const auto& entrances, storm::storage::EntranceExit entranceExit) {
        for (size_t i = 0; i < entrances.size(); ++i) {
            Position pos{entranceExit, i};
            std::pair<models::ConcreteMdp<ValueType>*, Position> key{ptr, pos};

            ParetoPointType zero(dimension, storm::utility::zero<storm::RationalNumber>());
            auto subDistributionPolytope = geometry::Polytope<ParetoRational>::getSubdistributionPolytope(dimension);

            lowerBounds[key] = {zero};
            upperBounds[key] = subDistributionPolytope;
        }
    };

    initializeEntrances(ptr->getLEntrance(), L_ENTRANCE);
    initializeEntrances(ptr->getREntrance(), R_ENTRANCE);
}

template<typename ValueType>
void ParetoCache<ValueType>::initializeParetoCurves(std::vector<models::ConcreteMdp<ValueType>*>& leaves) {
    for (models::ConcreteMdp<ValueType>* ptr : leaves) {
        initializeParetoCurve(ptr);
    }
}

template<typename ValueType>
bool ParetoCache<ValueType>::needScheduler() {
    return true;
}

template<typename ValueType>
std::pair<typename ParetoCache<ValueType>::ParetoPointType, typename ParetoCache<ValueType>::ParetoPointType> ParetoCache<ValueType>::getLowerUpper(
    models::ConcreteMdp<ValueType>* ptr, ParetoPointType outputWeight, Position pos) {
    std::pair<models::ConcreteMdp<ValueType>*, Position> key = {ptr, pos};

    auto upperBoundPolytope = upperBounds.at(key);

    ParetoPointType lb = getBestLowerBound(ptr, outputWeight, pos);

    static storm::utility::Stopwatch optimizeTimer;
    optimizeTimer.start();
    auto ub = upperBoundPolytope->optimize(outputWeight);
    optimizeTimer.stop();

    // std::cout << "UB hs: " << upperBoundPolytope->getHalfspaces().size() << std::endl;
    // std::cout << "Total itme Took: " << optimizeTimer << std::endl;

    STORM_LOG_ASSERT(ub.second, "optimizing in the upper bound should always be defined as it is (supposedly) bounded and non-empty");

    return std::make_pair(lb, ub.first);
}

template<typename ValueType>
typename ParetoCache<ValueType>::ParetoPointType ParetoCache<ValueType>::getBestLowerBound(models::ConcreteMdp<ValueType>* ptr, ParetoPointType outputWeight,
                                                                                           Position pos) {
    std::pair<models::ConcreteMdp<ValueType>*, Position> key = {ptr, pos};
    auto lowerBoundPoints = lowerBounds.at(key);
    STORM_LOG_ASSERT(lowerBoundPoints.size() > 0, "Empty lower bound");

    ParetoRational lbValue = storm::utility::zero<ValueType>();
    ParetoPointType lb;
    // Compute lb = argmax_p [p*w]
    for (const auto& p : lowerBoundPoints) {
        ParetoRational weightedSum = storm::utility::vector::dotProduct(p, outputWeight);
        if (weightedSum >= lbValue) {
            lb = p;
            lbValue = weightedSum;
        }
    }

    return lb;
}

template<typename ValueType>
typename ParetoCache<ValueType>::ParetoPointType ParetoCache<ValueType>::getBestUpperBound(models::ConcreteMdp<ValueType>* ptr, ParetoPointType outputWeight,
                                                                                           Position pos) {
    std::pair<models::ConcreteMdp<ValueType>*, Position> key = {ptr, pos};
    auto upperBoundPolytope = upperBounds.at(key);
    auto result = upperBoundPolytope->optimize(outputWeight);

    return result.first;
}

template<typename ValueType>
typename ParetoCache<ValueType>::ParetoRational ParetoCache<ValueType>::getError(ParetoPointType lower, ParetoPointType upper) const {
    STORM_LOG_ASSERT(lower.size() == upper.size(), "size mismatch " << lower.size() << " vs " << upper.size());

    ParetoRational maxError = -storm::utility::infinity<ParetoRational>();
    for (size_t i = 0; i < upper.size(); ++i) {
        ParetoRational error = upper[i] - lower[i];
        if (error > maxError) {
            maxError = error;
        }
    }

    return maxError;
}

template<typename ValueType>
std::shared_ptr<storm::models::ConcreteMdp<ValueType>> ParetoCache<ValueType>::toLowerBoundShortcutMdp(
    std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager, storm::models::ConcreteMdp<ValueType>* model) {
    auto reachabilityResult = getLowerBoundReachabilityResult(model);
    return reachabilityResult.toConcreteMdp(manager);
}

template<typename ValueType>
std::shared_ptr<storm::models::ConcreteMdp<ValueType>> ParetoCache<ValueType>::toUpperBoundShortcutMdp(
    std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager, storm::models::ConcreteMdp<ValueType>* model) {
    auto reachabilityResult = getUpperBoundReachabilityResult(model);
    return reachabilityResult.toConcreteMdp(manager);
}

template<typename ValueType>
size_t ParetoCache<ValueType>::getLowerParetoPointCount() {
    size_t total = 0;
    for (const auto& entry : lowerBounds) {
        total += entry.second.size();
    }

    return total;
}

template<typename ValueType>
size_t ParetoCache<ValueType>::getUpperParetoPointCount() {
    size_t total = 0;
    for (const auto& entry : upperBounds) {
        total += entry.second->getVertices().size();
    }

    return total;
}

template<typename ValueType>
storm::models::visitor::BidirectionalReachabilityResult<ValueType> ParetoCache<ValueType>::getLowerBoundReachabilityResult(
    storm::models::ConcreteMdp<ValueType>* model) {
    if (!isInitialized(model)) {
        initializeParetoCurve(model);
    }

    storm::models::visitor::BidirectionalReachabilityResult<ValueType> reachabilityResult(*model);

    auto processEntrances = [&](const auto& entrances, bool leftEntrance) {
        EntranceExit entranceExit = leftEntrance ? L_ENTRANCE : R_ENTRANCE;

        for (size_t entrance = 0; entrance < entrances.size(); ++entrance) {
            Position pos{entranceExit, entrance};
            const auto lb = lowerBounds.at({model, pos});

            for (const auto& point : lb) {
                auto convertedPoint = storm::utility::vector::convertNumericVector<ValueType>(point);
                reachabilityResult.addPoint(entrance, leftEntrance, convertedPoint);
            }
        }
    };

    processEntrances(model->getLEntrance(), true);
    processEntrances(model->getREntrance(), false);

    return reachabilityResult;
}

template<typename ValueType>
storm::models::visitor::BidirectionalReachabilityResult<ValueType> ParetoCache<ValueType>::getUpperBoundReachabilityResult(
    storm::models::ConcreteMdp<ValueType>* model) {
    if (!isInitialized(model)) {
        initializeParetoCurve(model);
    }

    storm::models::visitor::BidirectionalReachabilityResult<ValueType> reachabilityResult(*model);

    auto processEntrances = [&](const auto& entrances, bool leftEntrance) {
        EntranceExit entranceExit = leftEntrance ? L_ENTRANCE : R_ENTRANCE;

        for (size_t entrance = 0; entrance < entrances.size(); ++entrance) {
            Position pos{entranceExit, entrance};
            auto ub = upperBounds.at({model, pos});
            auto ubVertices = ub->getVertices();

            for (const auto& point : ubVertices) {
                auto convertedPoint = storm::utility::vector::convertNumericVector<ValueType>(point);
                reachabilityResult.addPoint(entrance, leftEntrance, convertedPoint);
            }
        }
    };

    processEntrances(model->getLEntrance(), true);
    processEntrances(model->getREntrance(), false);

    return reachabilityResult;
}

template<typename ValueType>
void ParetoCache<ValueType>::clearLowerBounds() {
    for (auto& entry : lowerBounds) {
        const auto& key = entry.first;
        auto& value = entry.second;

        models::ConcreteMdp<ValueType> const* ptr = key.first;
        size_t dimension = ptr->getExitCount();
        ParetoPointType zero(dimension, storm::utility::zero<storm::RationalNumber>());

        value = {zero};
    }
}

template<typename ValueType>
void ParetoCache<ValueType>::clearUpperBounds() {
    for (auto& entry : upperBounds) {
        const auto& key = entry.first;
        auto& value = entry.second;

        models::ConcreteMdp<ValueType> const* ptr = key.first;
        size_t dimension = ptr->getExitCount();
        value = geometry::Polytope<ParetoRational>::getSubdistributionPolytope(dimension);
    }
}

template<typename ValueType>
void ParetoCache<ValueType>::clear() {
    clearLowerBounds();
    clearUpperBounds();
}

template<typename ValueType>
bool ParetoCache<ValueType>::isValid() const {
    return true;
}

template<typename ValueType>
bool ParetoCache<ValueType>::containsLeaf(models::ConcreteMdp<ValueType>* ptr) const {
    return isInitialized(ptr);
}

template class ParetoCache<storm::RationalNumber>;
template class ParetoCache<double>;

}  // namespace storage
}  // namespace storm