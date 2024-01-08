#include "MappingVisitor.h"
#include "exceptions/InvalidArgumentException.h"
#include "exceptions/InvalidOperationException.h"
#include "exceptions/NotSupportedException.h"
#include "storm-compose/storage/EntranceExit.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
ValueVectorMapping<ValueType>::ValueVectorMapping(std::vector<ConcreteMdp<ValueType>*> leaves, std::map<Key, size_t> mapping, std::set<Key> outerPositions,
                                                  size_t highestIndex)
    : leaves(leaves), leafMapping(), mapping(mapping), modelMapping(leaves.size()), outerPositions(outerPositions), highestIndex(highestIndex) {
    for (size_t i = 0; i < leaves.size(); ++i) {
        leafMapping[leaves[i]] = i;
    }

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
std::set<std::pair<size_t, storage::Position>> const& ValueVectorMapping<ValueType>::getOuterPositions() const {
    return outerPositions;
}

template<typename ValueType>
std::vector<ConcreteMdp<ValueType>*>& ValueVectorMapping<ValueType>::getLeaves() {
    return leaves;
}

template<typename ValueType>
size_t ValueVectorMapping<ValueType>::getLeafId(ConcreteMdp<ValueType>* model) {
    return leafMapping[model];
}

template<typename ValueType>
void MappingVisitor<ValueType>::visitPrismModel(PrismModel<ValueType>& model) {
    STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "Concretize MDPs first");
}

template<typename ValueType>
void MappingVisitor<ValueType>::visitConcreteModel(ConcreteMdp<ValueType>& model) {
    localMapping = {};
    outerPositions = {};
    entranceExitStartIndices[currentLeafId] = {leftEntrancePos, rightEntrancePos, leftExitPos, rightExitPos};
    leaves.push_back(&model);

    auto processEntranceExit = [&](const auto& list, storage::EntranceExit entranceExit, auto& entranceExitPos) {
        for (size_t i = 0; i < list.size(); ++i) {
            storage::Position pos{entranceExit, entranceExitPos};
            localMapping.insert({{currentLeafId, pos}, localMapping.size()});
            outerPositions.insert({currentLeafId, pos});

            ++entranceExitPos;
        }
    };
    processEntranceExit(model.getLEntrance(), storage::L_ENTRANCE, leftEntrancePos);
    processEntranceExit(model.getREntrance(), storage::R_ENTRANCE, rightEntrancePos);
    processEntranceExit(model.getLExit(), storage::L_EXIT, leftExitPos);
    processEntranceExit(model.getRExit(), storage::R_EXIT, rightExitPos);

    ++currentLeafId;
}

template<typename ValueType>
void MappingVisitor<ValueType>::visitReference(Reference<ValueType>& reference) {
    auto openMdp = reference.getManager()->dereference(reference.getReference());
    openMdp->accept(*this);
}

template<typename ValueType>
void MappingVisitor<ValueType>::visitSequenceModel(SequenceModel<ValueType>& model) {
    STORM_LOG_THROW(model.getValues().size() > 0, storm::exceptions::InvalidArgumentException, "expected >0 children");

    std::map<std::pair<size_t, storage::Position>, size_t> mapping;
    std::set<std::pair<size_t, storage::Position>> outer;

    std::vector<decltype(mapping)> localMappings;
    std::vector<decltype(outer)> outerPositionsSets;

    // TODO rewrite so not all local mappings are stored at the same time for minor performance optimisation
    for (const auto& v : model.getValues()) {
        resetPos();
        v->accept(*this);
        localMappings.push_back(localMapping);
        outerPositionsSets.push_back(outerPositions);
    }

    mapping = localMappings[0];
    for (const auto& entry : mapping) {
        auto pos = entry.first.second;
        bool isLeft = pos.first == storage::L_ENTRANCE || pos.first == storage::L_EXIT;
        if (isLeft) {
            outer.insert(entry.first);
        }
    }

    for (size_t i = 1; i < localMappings.size(); ++i) {
        const auto& nextMapping = localMappings[i];
        const auto& nextOuter = outerPositionsSets[i];
        const auto& prevOuter = outerPositionsSets[i - 1];
        size_t offset = mapping.size();

        for (const auto& entry : nextMapping) {
            auto pos = entry.first.second;

            bool isLeft = pos.first == storage::L_ENTRANCE || pos.first == storage::L_EXIT;
            if (isLeft) {
                storage::Position matchedPosition = storage::positionMatch(pos);

                bool isOuter = nextOuter.count(entry.first) > 0;
                if (isOuter) {
                    // Map to correct shared index
                    bool inserted = false;
                    for (const auto& key : prevOuter) {
                        if (key.second == matchedPosition) {
                            mapping.insert({entry.first, mapping[key]});
                            inserted = true;
                            break;
                        }
                    }

                    STORM_LOG_ASSERT(inserted, "failed to insert");
                } else {
                    mapping.insert({entry.first, offset + entry.second});
                }
            } else {
                mapping.insert({entry.first, offset + entry.second});
            }
        }
    }

    for (const auto& entry : localMappings[localMappings.size() - 1]) {
        auto pos = entry.first.second;
        bool isRight = pos.first == storage::R_ENTRANCE || pos.first == storage::R_EXIT;
        if (isRight) {
            outer.insert(entry.first);
        }
    }

    localMapping = mapping;
    outerPositions = outer;
}

template<typename ValueType>
void MappingVisitor<ValueType>::visitSumModel(SumModel<ValueType>& model) {
    std::map<std::pair<size_t, storage::Position>, size_t> mapping;
    std::set<std::pair<size_t, storage::Position>> outer;

    for (const auto& v : model.getValues()) {
        v->accept(*this);

        // Add offset to each entry to make sure that entrances/exits of the values in Sum are disjoint
        size_t offset = mapping.size();
        for (const auto& entry : localMapping) {
            size_t newIndex = offset + entry.second;
            mapping.insert({entry.first, newIndex});

            if (outerPositions.count(entry.first) > 0) {
                outer.insert(entry.first);
            }
        }
    }

    localMapping = mapping;
    outerPositions = outer;
}

template<typename ValueType>
void MappingVisitor<ValueType>::visitTraceModel(TraceModel<ValueType>& model) {
    STORM_LOG_THROW(false, storm::exceptions::NotSupportedException, "Trace operator is currently not supported");
    model.getValue()->accept(*this);

    size_t left = model.getLeft();
    size_t right = model.getRight();

    // Map 0..left-1 exits to the same index as their right entrance counterpart
    for (size_t i = 0; i < left; ++i) {
    }

    // Map 0..right-1 exits to the same index as their left entrance counterpart
    for (size_t i = 0; i < right; ++i) {
    }
}

template<typename ValueType>
ValueVectorMapping<ValueType> MappingVisitor<ValueType>::getMapping() {
    size_t highestIndex = 0;
    for (const auto& entry : localMapping) {
        if (entry.second > highestIndex) {
            highestIndex = entry.second;
        }
    }

    return ValueVectorMapping<ValueType>(leaves, localMapping, outerPositions, highestIndex);
}

template<typename ValueType>
void MappingVisitor<ValueType>::performPostProcessing() {
    std::cout << "entranceExitStartIndices: " << std::endl;
    for (const auto& entry : entranceExitStartIndices) {
        size_t leafId = entry.first;
        const auto& tuple = entry.second;

        std::cout << leafId << " -> <" << std::get<0>(tuple) << ", " << std::get<1>(tuple) << ", " << std::get<2>(tuple) << ", " << std::get<3>(tuple) << ">"
                  << std::endl;
    }

    decltype(outerPositions) newOuterPositions;
    decltype(localMapping) newLocalMapping;

    for (auto& entry : localMapping) {
        const auto& key = entry.first;
        size_t leafId = key.first;
        storage::Position pos = key.second;
        std::cout << "leafId: " << leafId << ", pos: " << storage::positionToString(pos) << std::endl;

        const auto& offsets = entranceExitStartIndices[leafId];

        size_t lEnOffset, rEnOffset, lExOffset, rExOffset;
        std::tie(lEnOffset, rEnOffset, lExOffset, rExOffset) = offsets;

        size_t offset;
        if (pos.first == storage::L_ENTRANCE) {
            offset = lEnOffset;
        } else if (pos.first == storage::R_ENTRANCE) {
            offset = rEnOffset;
        } else if (pos.first == storage::L_EXIT) {
            offset = lExOffset;
        } else if (pos.first == storage::R_EXIT) {
            offset = rExOffset;
        } else {
            STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "sanity check failed");
        }
        size_t newIndex = pos.second - offset;
        std::cout << "NI: " << newIndex << std::endl;
        storage::Position newPosition{pos.first, newIndex};
        Key newKey{leafId, newPosition};

        newLocalMapping[newKey] = entry.second;

        bool isOuter = outerPositions.count(key) > 0;
        if (isOuter) {
            newOuterPositions.insert(newKey);
            std::cout << "nOuter: " << newKey.first << " " << storage::positionToString(newKey.second) << std::endl;
        }
    }

    localMapping = newLocalMapping;
    outerPositions = newOuterPositions;
}

template<typename ValueType>
void MappingVisitor<ValueType>::resetPos() {
    leftEntrancePos = 0;
    rightEntrancePos = 0;
    leftExitPos = 0;
    rightExitPos = 0;
}

template class MappingVisitor<double>;
template class MappingVisitor<storm::RationalNumber>;
template class ValueVectorMapping<double>;
template class ValueVectorMapping<storm::RationalNumber>;

}  // namespace visitor
}  // namespace models
}  // namespace storm
