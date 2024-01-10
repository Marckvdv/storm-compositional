#pragma once

#include <set>

#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/storage/EntranceExit.h"

namespace storm {
namespace storage {

template<typename ValueType>
class ValueVectorMapping {
    typedef std::pair<size_t, Position> Key;

   public:
    ValueVectorMapping(std::vector<models::ConcreteMdp<ValueType>*> leaves, std::map<Key, size_t> mapping, std::set<Key> outerPositions, size_t highestIndex);
    ValueVectorMapping() = default;

    void print() const;
    size_t lookup(const Key& key) const;
    size_t getHighestIndex() const;
    size_t getLeafCount() const;
    size_t size() const;
    std::set<Key> const& getOuterPositions() const;
    std::vector<models::ConcreteMdp<ValueType>*>& getLeaves();
    std::map<Position, size_t>& getModelMapping(size_t leafId);

   private:
    // All the leaves
    std::vector<models::ConcreteMdp<ValueType>*> leaves;

    // mapping from <leave_id, position> to value vector index
    std::map<Key, size_t> mapping;
    std::vector<std::map<Position, size_t>> modelMapping;
    std::set<Key> outerPositions;
    size_t highestIndex;
};

}  // namespace storage
}  // namespace storm
