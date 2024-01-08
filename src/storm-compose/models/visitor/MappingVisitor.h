#pragma once

#include "EntranceExitVisitor.h"
#include "OpenMdpVisitor.h"
#include "exceptions/InvalidOperationException.h"
#include "storm-compose/models/PrismModel.h"
#include "storm-compose/models/Reference.h"
#include "storm-compose/models/SumModel.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
class ValueVectorMapping {
    typedef std::pair<size_t, storage::Position> Key;

   public:
    ValueVectorMapping(std::vector<ConcreteMdp<ValueType>*> leaves, std::map<Key, size_t> mapping, std::set<Key> outerPositions, size_t highestIndex);
    ValueVectorMapping() = default;

    void print() const;
    size_t lookup(const Key& key) const;
    size_t getHighestIndex() const;
    size_t getLeafCount() const;
    std::set<Key> const& getOuterPositions() const;
    std::vector<ConcreteMdp<ValueType>*>& getLeaves();
    size_t getLeafId(ConcreteMdp<ValueType>* model);
    std::map<storage::Position, size_t>& getModelMapping(size_t leafId);

   private:
    // All the leaves
    std::vector<ConcreteMdp<ValueType>*> leaves;
    std::map<ConcreteMdp<ValueType>*, size_t> leafMapping;

    // mapping from <leave_id, position> to value vector index
    std::map<Key, size_t> mapping;
    std::vector<std::map<storage::Position, size_t>> modelMapping;
    std::set<Key> outerPositions;
    size_t highestIndex;
};

template<typename ValueType>
class MappingVisitor : public OpenMdpVisitor<ValueType> {
   public:
    MappingVisitor() {}
    virtual ~MappingVisitor() override {}

    virtual void visitPrismModel(PrismModel<ValueType>& model) override;
    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override;
    virtual void visitReference(Reference<ValueType>& reference) override;
    virtual void visitSequenceModel(SequenceModel<ValueType>& model) override;
    virtual void visitSumModel(SumModel<ValueType>& model) override;
    virtual void visitTraceModel(TraceModel<ValueType>& model) override;

    ValueVectorMapping<ValueType> getMapping();

   private:
    void resetPos();

    std::map<std::pair<size_t, storage::Position>, size_t> localMapping;
    std::set<std::pair<size_t, storage::Position>> outerPositions;
    std::vector<ConcreteMdp<ValueType>*> leaves;

    size_t leftEntrancePos = 0, rightEntrancePos = 0, leftExitPos = 0, rightExitPos = 0;
    size_t currentSequencePosition = 0;
    size_t currentLeafId = 0;
};

}  // namespace visitor
}  // namespace models
}  // namespace storm
