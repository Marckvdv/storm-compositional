#pragma once

#include "OpenMdpVisitor.h"
#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/models/OpenMdp.h"
#include "storm/storage/SparseMatrix.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
class FlatMdpBuilderVisitor : public OpenMdpVisitor<ValueType> {
   public:
    FlatMdpBuilderVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager);

    void visitPrismModel(PrismModel<ValueType>& model) override;
    void visitConcreteModel(ConcreteMdp<ValueType>& model) override;

    // We use the default implementation which simply dereferences and visits that
    // virtual void visitReference(Reference<ValueType>& reference) override;

    void visitSequenceModel(SequenceModel<ValueType>& model) override;
    void visitSumModel(SumModel<ValueType>& model) override;
    void visitTraceModel(TraceModel<ValueType>& model) override;

    ConcreteMdp<ValueType> getCurrent();

   protected:
    std::shared_ptr<OpenMdpManager<ValueType>> manager;
    // boost::optional<ConcreteMdp<ValueType>> current;
    ConcreteMdp<ValueType> current;
};

}  // namespace visitor
}  // namespace models
}  // namespace storm
