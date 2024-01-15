#include "ValueVectorMapping.h"
#include "storm-compose/storage/EntranceExit.h"

namespace storm {
namespace storage {

template<typename ValueType>
ValueVectorMapping<ValueType>::ValueVectorMapping(std::vector<models::ConcreteMdp<ValueType>*> leaves, std::map<Key, size_t> mapping,
                                                  std::set<Key> outerPositions, size_t highestIndex)
    : leaves(leaves), mapping(mapping), modelMapping(leaves.size()), outerPositions(outerPositions), highestIndex(highestIndex) {
    // Maps value vector indices to their exit
    std::map<size_t, Key> reverseMap;
    for (const auto& entry : mapping) {
        const auto& key = entry.first;
        const auto& value = entry.second;
        size_t leafId = key.first;
        storage::Position pos = key.second;

        if (pos.first == L_EXIT || pos.first == R_EXIT) {
            reverseMap[value] = key;
        }

        modelMapping[leafId][pos] = value;
    }

    // std::cout << "Mapping " << std::endl;
    //  Populate connectedMapping
    for (const auto& entry : mapping) {
        const auto& key = entry.first;
        const auto& value = entry.second;
        storage::Position pos = key.second;

        if (pos.first == L_ENTRANCE || pos.first == R_ENTRANCE) {
            const auto connectedKey = reverseMap[value];
            size_t connectedLeaf = connectedKey.first;
            connectedMapping[key] = connectedLeaf;

            // std::cout << "leafId " << key.first << " " << storage::positionToString(pos) << " -> " << connectedLeaf << std::endl;
        }
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

template<typename ValueType>
models::ConcreteMdp<ValueType> const* ValueVectorMapping<ValueType>::getLeaf(size_t leafId) const {
    return leaves[leafId];
}

template<typename ValueType>
boost::optional<size_t> ValueVectorMapping<ValueType>::getConnectedLeafId(size_t leafId, Position pos) {
    STORM_LOG_ASSERT(pos.first == L_ENTRANCE || pos.first == R_ENTRANCE, "Only supported for entrance positions");

    const Key key{leafId, pos};
    const auto it = connectedMapping.find(key);
    if (it == connectedMapping.end()) {
        return boost::none;
    } else {
        return connectedMapping[key];
    }
}

template class ValueVectorMapping<double>;
template class ValueVectorMapping<storm::RationalNumber>;

}  // namespace storage
}  // namespace storm
