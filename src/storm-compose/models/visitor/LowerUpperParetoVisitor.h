#pragma once

#include "OpenMdpVisitor.h"

#include "storm-compose/models/PrismModel.h"
#include "storm-compose/models/Reference.h"
#include "storm-compose/models/SequenceModel.h"
#include "storm-compose/models/SumModel.h"
#include "storm-compose/models/TraceModel.h"
#include "storm-compose/models/visitor/OpenMdpVisitor.h"
#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/models/visitor/BidirectionalReachabilityResult.h"
#include "storm-compose/benchmark/BenchmarkStats.h"
#include "storm/modelchecker/results/CheckResult.h"
#include "storm/environment/Environment.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType> class BidirectionalReachabilityResult;

struct LowerUpperParetoSettings {
    double precision;
    std::string precisionType;

    boost::optional<size_t> steps;
};

/*
Current workflow:
After visiting any node, a Pareto result is stored.
For sequence, we compute the Pareto result of the children, turn these into MDPs and compute the Pareto result of this MDP.
Similar for sum and trace.
*/

// The point of this visitor is that after visiting anything, we get pareto results back.
// No intermediate ConcreteMdps need to be stored.
template<typename ValueType>
class LowerUpperParetoVisitor : public OpenMdpVisitor<ValueType> {
    typedef std::pair<BidirectionalReachabilityResult<ValueType>, BidirectionalReachabilityResult<ValueType>> ParetoType;
public:
    LowerUpperParetoVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager, storm::compose::benchmark::BenchmarkStats<ValueType>& stats, LowerUpperParetoSettings settings);

    virtual void visitPrismModel(PrismModel<ValueType>& model) override;
    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override;
    virtual void visitReference(Reference<ValueType>& reference) override;
    virtual void visitSequenceModel(SequenceModel<ValueType>& model) override;
    virtual void visitSumModel(SumModel<ValueType>& model) override;
    virtual void visitTraceModel(TraceModel<ValueType>& model) override;

    ParetoType getCurrentPareto();

    // TODO Functions below are public because it is also being used in PropertyDrivenVisitor
    // Need to find some common place to store this instead.
    static std::string getFormula(PrismModel<ValueType> const& model, bool rewards=false);
    static std::string getFormula(ConcreteMdp<ValueType> const& model, bool rewards=false);

private:
    static std::unordered_map<std::string, storm::expressions::Expression> getIdentifierMapping(storm::expressions::ExpressionManager const& manager);

    std::unordered_map<std::string, ParetoType> paretoResults;
    ParetoType currentPareto;
    std::shared_ptr<OpenMdpManager<ValueType>> manager;
    storm::compose::benchmark::BenchmarkStats<ValueType>& stats;

    storm::Environment env;
    LowerUpperParetoSettings settings;
};

}
}
}
