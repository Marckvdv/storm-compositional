#pragma once

#include <map>
#include <vector>

#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/models/visitor/MappingVisitor.h"
#include "storm-compose/models/visitor/OpenMdpVisitor.h"
#include "storm-compose/storage/ValueVectorMapping.h"

namespace storm {
namespace storage {

template<typename ValueType>
class ValueVectorMapping;

template<typename ValueType>
class ValueVector {
   public:
    ValueVector(ValueVectorMapping<ValueType>&& mapping, std::vector<ValueType> finalWeight);
    ValueVector() = default;

    ValueType getWeight(size_t leafId, storage::Position position);
    void setWeight(size_t leafId, storage::Position position, ValueType value);
    void initializeValues();
    void addConstant(ValueType epsilon, bool clamp = true);
    std::vector<ValueType>& getValues();
    bool dominates(ValueVector<ValueType> const& other) const;
    bool dominatedBy(ValueVector<ValueType> const& other) const;
    ValueVectorMapping<ValueType>& getMapping();
    std::vector<ValueType> getOutputWeights(size_t leafId);
    void print();

   private:
    ValueVectorMapping<ValueType> mapping;
    std::vector<ValueType> values, finalWeight;
    std::set<size_t> exitIndices;
};

}  // namespace storage
}  // namespace storm
