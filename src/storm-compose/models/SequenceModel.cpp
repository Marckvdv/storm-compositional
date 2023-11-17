#include "SequenceModel.h"

namespace storm {
namespace models {

template<typename ValueType>
SequenceModel<ValueType>::SequenceModel(std::shared_ptr<OpenMdpManager<ValueType>> manager, std::vector<std::shared_ptr<OpenMdp<ValueType>>> values)
    : OpenMdp<ValueType>(manager), values(values) {}

template<typename ValueType>
void SequenceModel<ValueType>::accept(visitor::OpenMdpVisitor<ValueType>& visitor) {
    visitor.visitSequenceModel(*this);
}

template<typename ValueType>
std::vector<std::shared_ptr<OpenMdp<ValueType>>>& SequenceModel<ValueType>::getValues() {
    return values;
}

template<typename ValueType>
std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> SequenceModel<ValueType>::collectEntranceExit(
    typename OpenMdp<ValueType>::EntranceExit entranceExit, typename OpenMdp<ValueType>::Scope& scope) const {
    if (values.size() == 0) {
        STORM_LOG_ASSERT(false, "something went wrong");
        return {};
    } else if (values.size() == 1) {
        scope.pushScope(0);
        auto result = values[0]->collectEntranceExit(entranceExit, scope);
        scope.popScope();
        return result;
    }

    std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> entries;
    if (entranceExit == OpenMdp<ValueType>::L_ENTRANCE || entranceExit == OpenMdp<ValueType>::L_EXIT) {
        scope.pushScope(0);
        entries = values[0]->collectEntranceExit(entranceExit, scope);
        scope.popScope();
    } else {
        size_t idx = values.size() - 1;
        scope.pushScope(idx);
        entries = values[idx]->collectEntranceExit(entranceExit, scope);
        scope.popScope();
    }

    return entries;
}

template<typename ValueType>
bool SequenceModel<ValueType>::isRightward() const {
    for (const auto& value : values) {
        if (!value->isRightward()) {
            return false;
        }
    }
    return true;
}

template class SequenceModel<storm::RationalNumber>;
template class SequenceModel<double>;

}  // namespace models
}  // namespace storm
