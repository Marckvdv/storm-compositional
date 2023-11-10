#pragma once

#include "OpenMdpVisitor.h"
#include "storm/environment/Environment.h"

namespace storm {
namespace models {
namespace visitor {

template <typename ValueType>
class PropertyDrivenVisitor : public OpenMdpVisitor<ValueType> {
    typedef std::vector<ValueType> WeightType;
public:
    PropertyDrivenVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager);
    virtual ~PropertyDrivenVisitor();

    void visitPrismModel(PrismModel<ValueType>& model) override;
    void visitConcreteModel(ConcreteMdp<ValueType>& model) override;
    void visitReference(Reference<ValueType>& reference) override;
    void visitSequenceModel(SequenceModel<ValueType>& model) override;
    void visitSumModel(SumModel<ValueType>& model) override;
    void visitTraceModel(TraceModel<ValueType>& model) override;

    void setWeight(std::vector<ValueType> weight);
    void setTargetExit(size_t exitCount, size_t exit, bool leftExit);

    WeightType weightedReachability(WeightType weights, ConcreteMdp<ValueType> concreteMdp);
    WeightType getCurrentWeight();

private:
    WeightType currentWeight;
    storm::Environment env;
    std::shared_ptr<OpenMdpManager<ValueType>> manager;
    WeightType getOptimalWeight(WeightType weights, WeightType currentValues);
};

}
}
}