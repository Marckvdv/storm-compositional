#include "ValueVectorMapping.h"

namespace storm {
namespace storage {

template<typename ValueType>
ValueVectorMapping<ValueType>::ValueVectorMapping(std::vector<models::ConcreteMdp<ValueType>*> leaves, std::map<Key, size_t> mapping,
                                                  std::set<Key> outerPositions, size_t highestIndex)
    : leaves(leaves), mapping(mapping), modelMapping(leaves.size()), outerPositions(outerPositions), highestIndex(highestIndex) {
    for (const auto& entry : mapping) {
        const auto& key = entry.first;
        const auto& value = entry.second;

        size_t leafId = key.first;
        storage::Position pos = key.second;

        modelMapping[leafId][pos] = value;
    }
}

template<typename ValueType>
void ValueVectorMapping<ValueType>::print() const {
    for (const auto& entry : mapping) {
        size_t leafId = entry.first.first;
        storage::Position position = entry.first.second;
        size_t index = entry.second;

        std::cout << leaves[leafId]->getName() << " " << storage::positionToString(position) << " -> " << index << std::endl;
    }
}

template<typename ValueType>
size_t ValueVectorMapping<ValueType>::lookup(const Key& key) const {
    return mapping.at(key);
}

template<typename ValueType>
size_t ValueVectorMapping<ValueType>::getHighestIndex() const {
    return highestIndex;
}

template<typename ValueType>
size_t ValueVectorMapping<ValueType>::getLeafCount() const {
    return leaves.size();
}

template<typename ValueType>
size_t ValueVectorMapping<ValueType>::size() const {
    return mapping.size();
}

template<typename ValueType>
std::set<std::pair<size_t, storage::Position>> const& ValueVectorMapping<ValueType>::getOuterPositions() const {
    return outerPositions;
}

template<typename ValueType>
std::vector<models::ConcreteMdp<ValueType>*>& ValueVectorMapping<ValueType>::getLeaves() {
    return leaves;
}

template class ValueVectorMapping<double>;
template class ValueVectorMapping<storm::RationalNumber>;

}  // namespace storage
}  // namespace storm
