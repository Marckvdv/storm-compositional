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
#include "storm-compose/models/visitor/FlatMdpBuilderVisitor.h"
#include "storm/modelchecker/results/CheckResult.h"
#include "storm/environment/Environment.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType> class BidirectionalReachabilityResult;

/*
Current workflow:
After visiting any node, a Pareto result is stored.
For sequence, we compute the Pareto result of the children, turn these into MDPs and compute the Pareto result of this MDP.
Similar for sum and trace.
*/

// The point of this visitor is that after visiting anything, we get pareto results back.
// No intermediate ConcreteMdps need to be stored.
template<typename ValueType>
class ParetoVisitor : public OpenMdpVisitor<ValueType> {
public:
    ParetoVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager);

    virtual void visitPrismModel(PrismModel<ValueType>& model) override;
    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override;
    virtual void visitReference(Reference<ValueType>& reference) override;
    virtual void visitSequenceModel(SequenceModel<ValueType>& model) override;
    virtual void visitSumModel(SumModel<ValueType>& model) override;
    virtual void visitTraceModel(TraceModel<ValueType>& model) override;

    BidirectionalReachabilityResult<ValueType> getCurrentPareto();

    // TODO Functions below are public because it is also being used in PropertyDrivenVisitor
    // Need to find some common place to store this instead.
    static std::string getFormula(PrismModel<ValueType> const& model, bool rewards=false);
    static std::string getFormula(ConcreteMdp<ValueType> const& model, bool rewards=false);

private:
    static std::unordered_map<std::string, storm::expressions::Expression> getIdentifierMapping(storm::expressions::ExpressionManager const& manager);

    std::unordered_map<std::string, BidirectionalReachabilityResult<ValueType>> paretoResults;
    BidirectionalReachabilityResult<ValueType> currentPareto;
    std::shared_ptr<OpenMdpManager<ValueType>> manager;

    storm::Environment env;
};

}
}
}
