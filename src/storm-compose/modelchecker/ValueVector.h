#pragma once

#include <vector>
#include <map>

#include "storm-compose/models/visitor/OpenMdpVisitor.h"
#include "storm-compose/models/visitor/MappingVisitor.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
class ValueVector {
public:
    ValueVector(ValueVectorMapping<ValueType>&& mapping, std::vector<ValueType> finalWeight);
    ValueVector() = default;

    ValueType getWeight(size_t leafId, storage::Position position);
    void setWeight(size_t leafId, storage::Position position, ValueType value);
    void initializeValues();
    void addConstant(ValueType epsilon);
    std::vector<ValueType>& getValues();
    bool dominates(ValueVector<ValueType> const& other);

private:
    ValueVectorMapping<ValueType> mapping;
    std::vector<ValueType> values, finalWeight;
};

}  // namespace visitor
}  // namespace models
}  // namespace storm
