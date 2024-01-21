#pragma once

#include "OpenMdpVisitor.h"

#include "storm-compose/benchmark/BenchmarkStats.h"
#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/models/PrismModel.h"
#include "storm-compose/models/Reference.h"
#include "storm-compose/models/SequenceModel.h"
#include "storm-compose/models/SumModel.h"
#include "storm-compose/models/TraceModel.h"
#include "storm-compose/models/visitor/BidirectionalReachabilityResult.h"
#include "storm-compose/models/visitor/LowerUpperParetoVisitor.h"
#include "storm-compose/models/visitor/OpenMdpVisitor.h"
#include "storm-compose/storage/ParetoCache.h"
#include "storm/environment/Environment.h"
#include "storm/modelchecker/results/CheckResult.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
class BidirectionalReachabilityResult;

/*
Current workflow:
After visiting any node, a Pareto result is stored.
For sequence, we compute the Pareto result of the children, turn these into MDPs and compute the Pareto result of this MDP.
Similar for sum and trace.
*/

// The point of this visitor is that after visiting anything, we get pareto results back.
// No intermediate ConcreteMdps need to be stored.
template<typename ValueType>
class SymbioticTermination : public OpenMdpVisitor<ValueType> {
    typedef std::pair<BidirectionalReachabilityResult<ValueType>, BidirectionalReachabilityResult<ValueType>> ParetoType;

   public:
    SymbioticTermination(std::shared_ptr<OpenMdpManager<ValueType>> manager, storm::compose::benchmark::BenchmarkStats<ValueType>& stats,
                         LowerUpperParetoSettings settings, storage::ParetoCache<ValueType>& paretoCache);

    virtual void visitPrismModel(PrismModel<ValueType>& model) override;
    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override;
    virtual void visitReference(Reference<ValueType>& reference) override;
    virtual void visitSequenceModel(SequenceModel<ValueType>& model) override;
    virtual void visitSumModel(SumModel<ValueType>& model) override;
    virtual void visitTraceModel(TraceModel<ValueType>& model) override;

    ParetoType getCurrentPareto();

    std::unique_ptr<storm::modelchecker::CheckResult> performMultiObjectiveModelChecking(storm::Environment env,
                                                                                         storm::models::sparse::Mdp<ValueType> const& mdp,
                                                                                         storm::logic::MultiObjectiveFormula const& formula);
    // static std::pair<ConcreteMdp<ValueType>, ConcreteMdp<ValueType>> toShortcutMdp(const ConcreteMdp<ValueType>& model, storm::Environment const& env);

   private:
    static std::unordered_map<std::string, storm::expressions::Expression> getIdentifierMapping(storm::expressions::ExpressionManager const& manager);

    std::unordered_map<std::string, ParetoType> paretoResults;
    ParetoType currentPareto;
    std::shared_ptr<OpenMdpManager<ValueType>> manager;
    storm::compose::benchmark::BenchmarkStats<ValueType>& stats;

    storm::Environment env;
    LowerUpperParetoSettings settings;
    storage::ParetoCache<ValueType>& paretoCache;
};

}  // namespace visitor
}  // namespace models
}  // namespace storm
