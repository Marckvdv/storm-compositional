#pragma once

#include "EntranceExitVisitor.h"
#include "OpenMdpVisitor.h"
#include "exceptions/InvalidOperationException.h"
#include "storm-compose/models/PrismModel.h"
#include "storm-compose/models/Reference.h"
#include "storm-compose/models/SumModel.h"
#include "storm-compose/storage/ValueVectorMapping.h"

namespace storm {

namespace storage {
template<typename ValueType>
class ValueVectorMapping;

}

namespace models {
namespace visitor {

template<typename ValueType>
class MappingVisitor : public OpenMdpVisitor<ValueType> {
    typedef std::pair<size_t, storage::Position> Key;

   public:
    MappingVisitor() {}
    virtual ~MappingVisitor() override {}

    virtual void visitPrismModel(PrismModel<ValueType>& model) override;
    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override;
    virtual void visitReference(Reference<ValueType>& reference) override;
    virtual void visitSequenceModel(SequenceModel<ValueType>& model) override;
    virtual void visitSumModel(SumModel<ValueType>& model) override;
    virtual void visitTraceModel(TraceModel<ValueType>& model) override;

    storage::ValueVectorMapping<ValueType> getMapping();
    void performPostProcessing();

   private:
    void resetPos();

    std::map<Key, size_t> localMapping;
    std::set<Key> outerPositions;

    /// Maps leafId -> <lEntranceStart, rEntranceStart, lExitStart, rExitStart>
    std::map<size_t, std::tuple<size_t, size_t, size_t, size_t>> entranceExitStartIndices;
    std::vector<ConcreteMdp<ValueType>*> leaves;

    size_t leftEntrancePos = 0, rightEntrancePos = 0, leftExitPos = 0, rightExitPos = 0;
    size_t currentSequencePosition = 0;
    size_t currentLeafId = 0;
};

}  // namespace visitor
}  // namespace models
}  // namespace storm
