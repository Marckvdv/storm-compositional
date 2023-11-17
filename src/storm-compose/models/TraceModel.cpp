#include "TraceModel.h"

namespace storm {
namespace models {

template<typename ValueType>
TraceModel<ValueType>::TraceModel(std::shared_ptr<OpenMdpManager<ValueType>> manager, std::shared_ptr<OpenMdp<ValueType>> value, size_t left, size_t right)
    : OpenMdp<ValueType>(manager), value(value), left(left), right(right) {}

template<typename ValueType>
void TraceModel<ValueType>::accept(visitor::OpenMdpVisitor<ValueType>& visitor) {
    visitor.visitTraceModel(*this);
}

template<typename ValueType>
std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> TraceModel<ValueType>::collectEntranceExit(
    typename OpenMdp<ValueType>::EntranceExit entranceExit, typename OpenMdp<ValueType>::Scope& scope) const {
    auto lEntrances = value->collectEntranceExit(OpenMdp<ValueType>::L_ENTRANCE, scope);
    auto rEntrances = value->collectEntranceExit(OpenMdp<ValueType>::R_ENTRANCE, scope);
    auto lExits = value->collectEntranceExit(OpenMdp<ValueType>::L_EXIT, scope);
    auto rExits = value->collectEntranceExit(OpenMdp<ValueType>::R_EXIT, scope);

    // check arity
    bool leftArityCorrect = lExits.size() >= left && rEntrances.size() >= left;
    bool rightArityCorrect = rExits.size() >= right && lEntrances.size() >= right;
    bool arityCorrect = leftArityCorrect && rightArityCorrect;
    STORM_LOG_ASSERT(arityCorrect, "arity mismatch!");

    std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> entries;

    if (entranceExit == OpenMdp<ValueType>::L_ENTRANCE) {
        entries.insert(std::begin(entries), std::begin(lEntrances) + right, std::end(lEntrances));
    } else if (entranceExit == OpenMdp<ValueType>::R_ENTRANCE) {
        entries.insert(std::begin(entries), std::begin(rEntrances) + left, std::end(rEntrances));
    } else if (entranceExit == OpenMdp<ValueType>::L_EXIT) {
        entries.insert(std::begin(entries), std::begin(lExits) + left, std::end(lExits));
    } else if (entranceExit == OpenMdp<ValueType>::R_EXIT) {
        entries.insert(std::begin(entries), std::begin(rExits) + right, std::end(rExits));
    }
    return entries;
}

template<typename ValueType>
bool TraceModel<ValueType>::isRightward() const {
    if (left > 0) {
        return false;
    }

    return value->isRightward();
}

template<typename ValueType>
std::shared_ptr<OpenMdp<ValueType>> TraceModel<ValueType>::getValue() {
    return value;
}

template<typename ValueType>
size_t TraceModel<ValueType>::getLeft() {
    return left;
}

template<typename ValueType>
size_t TraceModel<ValueType>::getRight() {
    return right;
}

template class TraceModel<storm::RationalNumber>;
template class TraceModel<double>;

}  // namespace models
}  // namespace storm
