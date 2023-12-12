#include "EntranceExitMappingVisitor.h"
#include "exceptions/InvalidOperationException.h"
#include "storm-compose/storage/EntranceExit.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
void EntranceExitMappingVisitor<ValueType>::visitPrismModel(PrismModel<ValueType>& model) {
    STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "Concretize MDPs first");
}

template<typename ValueType>
void EntranceExitMappingVisitor<ValueType>::visitConcreteModel(ConcreteMdp<ValueType>& model) {
    for (size_t i = 0; i < model.getLEntrance().size(); ++i) {
        // mapping.insert({{currentComponentId, {L_ENTRANCE, i}}, mapping.size()});

        scopeMapping.insert({{currentScope, {storage::L_ENTRANCE, currentLeftPosition}}, scopeMapping.size()});
        ++currentLeftPosition;
    }
    for (size_t i = 0; i < model.getREntrance().size(); ++i) {
        // mapping.insert({{currentComponentId, {R_ENTRANCE, i}}, mapping.size()});

        scopeMapping.insert({{currentScope, {storage::R_ENTRANCE, currentRightPosition}}, scopeMapping.size()});
        ++currentRightPosition;
    }

    //++currentComponentId;
}

template<typename ValueType>
void EntranceExitMappingVisitor<ValueType>::visitReference(Reference<ValueType>& reference) {
    auto openMdp = reference.getManager()->dereference(reference.getReference());
    openMdp->accept(*this);
}

template<typename ValueType>
void EntranceExitMappingVisitor<ValueType>::visitSequenceModel(SequenceModel<ValueType>& model) {
    currentSequencePosition = 0;

    auto& values = model.getValues();
    for (const auto& v : values) {
        currentLeftPosition = 0;
        currentRightPosition = 0;

        currentScope.pushScope(currentSequencePosition);
        v->accept(*this);
        currentScope.popScope();
        ++currentSequencePosition;
    }
}

template<typename ValueType>
void EntranceExitMappingVisitor<ValueType>::visitSumModel(SumModel<ValueType>& model) {
    auto& values = model.getValues();
    for (const auto& v : values) {
        v->accept(*this);
    }
}

template<typename ValueType>
void EntranceExitMappingVisitor<ValueType>::visitTraceModel(TraceModel<ValueType>& model) {
    STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "Not yet supported");
}

// template <typename ValueType>
// std::map<std::pair<size_t, Position>, size_t>& EntranceExitMappingVisitor<ValueType>::getMapping() {
//     return mapping;
// }

template<typename ValueType>
std::map<std::pair<Scope, storage::Position>, size_t>& EntranceExitMappingVisitor<ValueType>::getScopeMapping() {
    return scopeMapping;
}

// template <typename ValueType>
// boost::optional<size_t> EntranceExitMappingVisitor<ValueType>::getIndex(size_t componentId, Position position) const {
//     const auto it = mapping.find({componentId, position});
//     if (it != mapping.end()) {
//         return it->second;
//     } else {
//         return boost::none;
//     }
// }

// template <typename ValueType>
// boost::optional<size_t> EntranceExitMappingVisitor<ValueType>::getIndex(Scope scope, Position position) const {
//     if (position.first == L_ENTRANCE || position.first == R_ENTRANCE) {
//         const auto it = scopeMapping.find({scope, position});
//         STORM_LOG_THROW(it != scopeMapping.end(), storm::exceptions::InvalidOperationException, "index not found");
//
//         return it->second;
//     } else { // L_EXIT || R_EXIT
//         if (position.first == L_EXIT) {
//             scope.decreaseScope();
//         } else { // R_EXIT
//             scope.increaseScope();
//         }
//         const auto it = scopeMapping.find({scope, {match(position.first), position.second}});
//         if (it != scopeMapping.end()) {
//             return it->second;
//         } else {
//             return boost::none;
//         }
//     }
// }

// template <typename ValueType>
// size_t EntranceExitMappingVisitor<ValueType>::getWeightIndex(size_t componentId, Position position) const {
//     const auto it = weightMapping.find({componentId, position});
//     STORM_LOG_THROW(it != weightMapping.end(), storm::exceptions::InvalidOperationException, "index not found");
//     return it->second;
// }

// template <typename ValueType>
// size_t EntranceExitMappingVisitor<ValueType>::getEntranceCount() const {
//     return mapping.size();
// }

// template <typename ValueType>
// void EntranceExitMappingVisitor<ValueType>::setWeightMapping(std::map<std::pair<size_t, Position>, size_t> weightMapping) {
//     this->weightMapping = weightMapping;
// }

}  // namespace visitor
}  // namespace models
}  // namespace storm
