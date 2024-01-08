#pragma once

#include "storm-compose/benchmark/BenchmarkStats.h"
#include "storm-compose/modelchecker/AbstractOpenMdpChecker.h"
#include "storm-compose/modelchecker/ApproximateReachabilityResult.h"
#include "storm-compose/models/OpenMdp.h"
#include "storm-compose/models/visitor/OpenMdpVisitor.h"
#include "storm-compose/storage/ParetoCache.h"
#include "storm/environment/Environment.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
class BottomUpTermination : public OpenMdpVisitor<ValueType> {
   public:
    BottomUpTermination(std::shared_ptr<OpenMdpManager<ValueType>> manager, storm::compose::benchmark::BenchmarkStats<ValueType>& stats,
                        storm::Environment const& env, storm::storage::ParetoCache<ValueType>& cache);

    virtual void visitPrismModel(PrismModel<ValueType>& model) override;
    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override;

    storm::modelchecker::ApproximateReachabilityResult<ValueType> getReachabilityResult(storm::modelchecker::OpenMdpReachabilityTask task,
                                                                                        storm::models::OpenMdp<ValueType>& openMdp);

   private:
    std::map<ConcreteMdp<ValueType>*, std::shared_ptr<ConcreteMdp<ValueType>>> lowerBounds, upperBounds;
    storm::Environment env;
    std::shared_ptr<OpenMdpManager<ValueType>> manager;
    storm::storage::ParetoCache<ValueType>& cache;
};

}  // namespace visitor
}  // namespace models
}  // namespace storm
