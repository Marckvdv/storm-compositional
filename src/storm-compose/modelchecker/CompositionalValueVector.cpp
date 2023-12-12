#include "CompositionalValueVector.h"
#include "storm-compose/storage/EntranceExit.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
CompositionalValueVector<ValueType>::CompositionalValueVector(std::map<std::pair<Scope, storage::Position>, size_t> scopeMapping, std::vector<ValueType> finalWeight) : scopeMapping(scopeMapping), finalWeight(finalWeight) {
}

template<typename ValueType>
void CompositionalValueVector<ValueType>::visitPrismModel(PrismModel<ValueType>& model) {
    STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "Concretize MDPs first");
}

template<typename ValueType>
void CompositionalValueVector<ValueType>::visitConcreteModel(ConcreteMdp<ValueType>& model) {
    for (size_t i = 0; i < model.getLExit().size(); ++i) {
        currentScope.decreaseScope();
        const auto it = scopeMapping.find({currentScope, {storage::R_ENTRANCE, currentLeftPosition}});
        currentScope.increaseScope();

        bool outerExit = it == scopeMapping.end();
        size_t idx;
        if (outerExit) {
            idx = scopeMapping.size();
            values.resize(idx+1, 0);
            values[idx] = finalWeight[currentOuterExit++];
        } else {
            idx = it->second;
        }

        scopeMapping.insert({{currentScope, {storage::L_EXIT, currentLeftPosition}}, idx});

        ++currentLeftPosition;
    }
    for (size_t i = 0; i < model.getRExit().size(); ++i) {
        currentScope.increaseScope();
        const auto it = scopeMapping.find({currentScope, {storage::L_ENTRANCE, currentRightPosition}});
        currentScope.decreaseScope();

        bool outerExit = it == scopeMapping.end();
        size_t idx;
        if (outerExit) {
            idx = scopeMapping.size();
            values.resize(idx+1, 0);
            values[idx] = finalWeight[currentOuterExit++];
        } else {
            idx = it->second;
        }
        scopeMapping.insert({{currentScope, {storage::R_EXIT, currentRightPosition}}, idx});

        ++currentRightPosition;
    }
}

template<typename ValueType>
void CompositionalValueVector<ValueType>::visitReference(Reference<ValueType>& reference) {
    auto openMdp = reference.getManager()->dereference(reference.getReference());
    openMdp->accept(*this);
}

template<typename ValueType>
void CompositionalValueVector<ValueType>::visitSequenceModel(SequenceModel<ValueType>& model) {
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
void CompositionalValueVector<ValueType>::visitSumModel(SumModel<ValueType>& model) {
    auto& values = model.getValues();
    for (const auto& v : values) {
        v->accept(*this);
    }
}

template<typename ValueType>
void CompositionalValueVector<ValueType>::visitTraceModel(TraceModel<ValueType>& model) {
    STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "Not yet supported");
}


template<typename ValueType>
ValueType CompositionalValueVector<ValueType>::getWeight(Scope scope, storage::Position position) {
    auto& weight = values[scopeMapping.at({scope, position})];
    //std::cout << "got weight: " << weight << std::endl;
    return weight;
}

template<typename ValueType>
void CompositionalValueVector<ValueType>::setWeight(Scope scope, storage::Position position, ValueType value) {
    values[scopeMapping.at({scope, position})] = value;
}

template<typename ValueType>
void CompositionalValueVector<ValueType>::printMapping() {
    for (const auto &entry : scopeMapping) {
        const auto& scope = entry.first.first;
        const auto& direction = entry.first.second.first;
        const auto& position = entry.first.second.second;
        const auto& idx = entry.second;

        std::cout << "< ";
        for (const auto& v : scope.scope) {
            std::cout << v << " ";
        }
        std::cout << "> ";

        std::cout << storage::entranceExitToString(direction) << " " << position << " -> " << idx << std::endl;
    }
}

template<typename ValueType>
void CompositionalValueVector<ValueType>::initializeValues() {
    values.resize(scopeMapping.size(), 0);
}

template<typename ValueType>
std::vector<ValueType>& CompositionalValueVector<ValueType>::getValues() {
    return values;
}

template<typename ValueType>
void CompositionalValueVector<ValueType>::addConstant(ValueType epsilon) {
    for (auto& v : values) {
        v += epsilon;
    }
}

template<typename ValueType>
bool CompositionalValueVector<ValueType>::dominates(CompositionalValueVector<ValueType> const& other) {
    for (size_t i = 0; i < values.size(); ++i) {
        if (values[i] < other.values[i]) {
            return false;
        }
    }
    return true;
}

template class CompositionalValueVector<double>;
template class CompositionalValueVector<storm::RationalNumber>;

}  // namespace visitor
}  // namespace models
}  // namespace storm
