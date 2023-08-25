#pragma once

#include "storm-compose/models/OpenMdp.h"
#include "storm-compose/models/ConcreteMdp.h"
#include "storm/storage/SparseMatrix.h"
#include "OpenMdpVisitor.h"

namespace storm {
namespace models {
namespace visitor {

template <typename ValueType>
class FlatMdpBuilderVisitor : public OpenMdpVisitor<ValueType> {
public:
    FlatMdpBuilderVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager);

    virtual void visitPrismModel(PrismModel<ValueType>& model) override;
    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override;

    // We use the default implementation which simply dereferences and visits that
    //virtual void visitReference(Reference<ValueType>& reference) override;

    virtual void visitSequenceModel(SequenceModel<ValueType>& model) override;
    virtual void visitSumModel(SumModel<ValueType>& model) override;
    virtual void visitTraceModel(TraceModel<ValueType>& model) override;

    ConcreteMdp<ValueType> getCurrent();

protected:
    std::shared_ptr<OpenMdpManager<ValueType>> manager;
    //boost::optional<ConcreteMdp<ValueType>> current;
    ConcreteMdp<ValueType> current;
};

}
}
}
