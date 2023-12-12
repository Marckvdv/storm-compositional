#pragma once

#include <map>
#include <vector>

#include "storm-compose/models/visitor/EntranceExitVisitor.h"
#include "storm-compose/models/visitor/OpenMdpVisitor.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
class CompositionalValueVector : public OpenMdpVisitor<ValueType> {
   public:
    CompositionalValueVector(std::map<std::pair<Scope, storage::Position>, size_t> scopeMapping, std::vector<ValueType> finalWeight);
    CompositionalValueVector() = default;

    virtual void visitPrismModel(PrismModel<ValueType>& model) override;
    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override;
    virtual void visitReference(Reference<ValueType>& reference) override;
    virtual void visitSequenceModel(SequenceModel<ValueType>& model) override;
    virtual void visitSumModel(SumModel<ValueType>& model) override;
    virtual void visitTraceModel(TraceModel<ValueType>& model) override;

    ValueType getWeight(Scope scope, storage::Position position);
    void setWeight(Scope scope, storage::Position position, ValueType value);
    void printMapping();
    void initializeValues();
    void addConstant(ValueType epsilon);
    std::vector<ValueType>& getValues();
    bool dominates(CompositionalValueVector<ValueType> const& other);

   private:
    std::map<std::pair<Scope, storage::Position>, size_t>
        scopeMapping;  // initialized by constructor, initially populated with entrances only, updated with exits
    std::vector<ValueType> values, finalWeight;

    // std::vector<ConcreteMdp<ValueType>&> leaves;

    Scope currentScope;
    size_t currentLeftPosition = 0, currentRightPosition = 0;
    size_t currentSequencePosition = 0;
    size_t currentOuterExit = 0;
};

}  // namespace visitor
}  // namespace models
}  // namespace storm
