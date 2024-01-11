#pragma once

#include "AbstractOpenMdpChecker.h"
#include "storm-compose/benchmark/BenchmarkStats.h"
#include "storm-compose/models/visitor/CVIVisitor.h"
#include "storm-compose/models/visitor/EntranceExitMappingVisitor.h"
#include "storm-compose/models/visitor/EntranceExitVisitor.h"
#include "storm-compose/storage/AbstractCache.h"
#include "storm-compose/storage/ParetoCache.h"
#include "storm-compose/storage/ValueVector.h"
#include "storm/environment/Environment.h"
#include "storm/utility/constants.h"

// #include "storm/storage/geometry/NativePolytope.h"

namespace storm {

namespace models {
namespace visitor {
template<typename ValueType>
class CVIVisitor;
}
}  // namespace models

namespace modelchecker {

enum CacheMethod {
    NO_CACHE,
    EXACT_CACHE,
    PARETO_CACHE,
};

template<typename ValueType>
class CompositionalValueIteration : public AbstractOpenMdpChecker<ValueType> {
    friend class models::visitor::CVIVisitor<ValueType>;

   public:
    struct Options {
        size_t maxSteps = 200;
        ValueType epsilon = 1e-4;
        bool useOvi = true;
        bool useBottomUp = true;
        size_t oviInterval = 10;
        size_t bottomUpInterval = 10;

        CacheMethod cacheMethod = PARETO_CACHE;
        ValueType cacheErrorTolerance = storm::utility::convertNumber<ValueType>(0.1);
        std::string iterationOrder = "backward";
    };

    CompositionalValueIteration(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager, storm::compose::benchmark::BenchmarkStats<ValueType>& stats,
                                Options options);

    virtual ApproximateReachabilityResult<ValueType> check(OpenMdpReachabilityTask task) override;

   private:
    void initialize(OpenMdpReachabilityTask const& task);
    void initializeCache();
    void initializeParetoCurves();
    bool shouldTerminate();
    bool shouldCheckOVITermination();
    bool shouldCheckBottomUpTermination();
    bool isUpperbound(std::vector<ValueType> valueVector);

    size_t currentStep = 0;
    Options options;
    storage::ValueVector<ValueType> valueVector;
    std::shared_ptr<storm::storage::AbstractCache<ValueType>> cache;
    // std::shared_ptr<storm::storage::ParetoCache<ValueType>> cache;
    storm::Environment env;
};

}  // namespace modelchecker
}  // namespace storm
