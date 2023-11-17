#pragma once

#include "OpenMdpVisitor.h"
#include "storm-compose/benchmark/BenchmarkStats.h"

namespace storm {
namespace models {
namespace visitor {

template<class ValueType>
class BenchmarkStatsVisitor : public OpenMdpVisitor<ValueType> {
   public:
    BenchmarkStatsVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager, storm::compose::benchmark::BenchmarkStats<ValueType>& stats);

    virtual void visitPrismModel(PrismModel<ValueType>& model) override;
    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override;
    virtual void visitReference(Reference<ValueType>& reference) override;
    virtual void visitSequenceModel(SequenceModel<ValueType>& model) override;
    virtual void visitSumModel(SumModel<ValueType>& model) override;
    virtual void visitTraceModel(TraceModel<ValueType>& model) override;

   private:
    std::shared_ptr<OpenMdpManager<ValueType>> manager;
    storm::compose::benchmark::BenchmarkStats<ValueType>& stats;

    void increaseDepth();
    void decreaseDepth();

    std::set<std::string> uniqueLeaves;
    bool visitingUniqueLeave = false;
    size_t depth = 0;
};

}  // namespace visitor
}  // namespace models
}  // namespace storm