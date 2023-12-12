#include "ValueVector.h"
#include "storm-compose/models/visitor/MappingVisitor.h"
#include "storm-compose/storage/EntranceExit.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
ValueVector<ValueType>::ValueVector(ValueVectorMapping<ValueType>&& mapping, std::vector<ValueType> finalWeight)
    : mapping(mapping), values(mapping.getHighestIndex() + 1), finalWeight(finalWeight) {}

template<typename ValueType>
ValueType ValueVector<ValueType>::getWeight(size_t leafId, storage::Position position) {
    auto& weight = values[mapping.lookup({leafId, position})];
    // std::cout << "got weight: " << weight << std::endl;
    return weight;
}

template<typename ValueType>
void ValueVector<ValueType>::setWeight(size_t leafId, storage::Position position, ValueType value) {
    values[mapping.lookup({leafId, position})] = value;
}

template<typename ValueType>
void ValueVector<ValueType>::initializeValues() {
    auto outerPositions = mapping.getOuterPositions();

    size_t index = 0;
    for (const auto& entry : outerPositions) {
        storm::storage::EntranceExit entranceExit = entry.second.first;
        if (entranceExit == storage::L_ENTRANCE || entranceExit == storage::R_ENTRANCE)
            continue;
        std::cout << "Outer: " << entry.first << " " << storage::positionToString(entry.second) << std::endl;

        setWeight(entry.first, entry.second, finalWeight[index]);
        ++index;  // TODO make sure this lines up
    }
}

template<typename ValueType>
std::vector<ValueType>& ValueVector<ValueType>::getValues() {
    return values;
}

template<typename ValueType>
void ValueVector<ValueType>::addConstant(ValueType epsilon) {
    for (auto& v : values) {
        v += epsilon;
    }
}

template<typename ValueType>
bool ValueVector<ValueType>::dominates(ValueVector<ValueType> const& other) {
    for (size_t i = 0; i < values.size(); ++i) {
        if (values[i] < other.values[i]) {
            return false;
        }
    }
    return true;
}

template class ValueVector<double>;
template class ValueVector<storm::RationalNumber>;

}  // namespace visitor
}  // namespace models
}  // namespace storm
