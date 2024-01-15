#include "ValueVector.h"
#include "storm-compose/models/visitor/MappingVisitor.h"
#include "storm-compose/storage/EntranceExit.h"

namespace storm {
namespace storage {

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
    const auto& outerPositions = mapping.getOuterPositions();

    size_t index = 0;
    for (const auto& entry : outerPositions) {
        storm::storage::EntranceExit entranceExit = entry.second.first;
        if (entranceExit == storage::L_ENTRANCE || entranceExit == storage::R_ENTRANCE)
            continue;
        std::cout << "Outer: " << entry.first << " " << storage::positionToString(entry.second) << std::endl;
        size_t valueIndex = mapping.lookup(entry);
        values[valueIndex] = finalWeight[index];
        exitIndices.insert(valueIndex);

        ++index;  // TODO make sure this lines up
    }
}

template<typename ValueType>
std::vector<ValueType>& ValueVector<ValueType>::getValues() {
    return values;
}

template<typename ValueType>
void ValueVector<ValueType>::addConstant(ValueType epsilon, bool clamp) {
    for (size_t i = 0; i < values.size(); ++i) {
        // Do not add epsilon to (outer) exits
        if (exitIndices.count(i) > 0) {
            std::cout << "Skipping index: " << i << std::endl;
            continue;
        }

        auto& v = values[i];
        if (clamp) {
            v = storm::utility::min<ValueType>(v + epsilon, storm::utility::one<ValueType>());
        } else {
            v += epsilon;
        }
    }
}

template<typename ValueType>
bool ValueVector<ValueType>::dominates(ValueVector<ValueType> const& other) const {
    bool dominates = true;
    for (size_t i = 0; i < values.size(); ++i) {
        std::cout << values[i] << " -> " << other.values[i] << " " << i << "/" << values.size();

        if (values[i] < other.values[i]) {
            dominates = false;
            std::cout << ", no OVI";
            // break;
        }
        std::cout << std::endl;
    }
    return dominates;
}

template<typename ValueType>
bool ValueVector<ValueType>::dominatedBy(ValueVector<ValueType> const& other) const {
    return other.dominates(*this);
}

template<typename ValueType>
ValueVectorMapping<ValueType>& ValueVector<ValueType>::getMapping() {
    return mapping;
}

template<typename ValueType>
std::vector<ValueType> ValueVector<ValueType>::getOutputWeights(size_t leafId) {
    std::vector<ValueType> weights;
    models::ConcreteMdp<ValueType>* model = mapping.getLeaves()[leafId];

    for (size_t leftPos = 0; leftPos < model->getLExit().size(); ++leftPos) {
        std::pair<storage::EntranceExit, size_t> pos{storage::L_EXIT, leftPos};
        ValueType weight = getWeight(leafId, pos);
        weights.push_back(weight);
    }

    for (size_t rightPos = 0; rightPos < model->getRExit().size(); ++rightPos) {
        std::pair<storage::EntranceExit, size_t> pos{storage::R_EXIT, rightPos};
        ValueType weight = getWeight(leafId, pos);
        weights.push_back(weight);
    }

    return weights;
}

template<typename ValueType>
void ValueVector<ValueType>::print() {
    mapping.printValueVector(*this);
}

template class ValueVector<double>;
template class ValueVector<storm::RationalNumber>;

}  // namespace storage
}  // namespace storm
