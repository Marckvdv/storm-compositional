#include "SumModel.h"

namespace storm {
namespace models {

template<typename ValueType>
SumModel<ValueType>::SumModel(std::shared_ptr<OpenMdpManager<ValueType>> manager, std::vector<std::shared_ptr<OpenMdp<ValueType>>> values)
    : OpenMdp<ValueType>(manager), values(values) {}

template<typename ValueType>
void SumModel<ValueType>::accept(visitor::OpenMdpVisitor<ValueType>& visitor) {
    visitor.visitSumModel(*this);
}

template<typename ValueType>
std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> SumModel<ValueType>::collectEntranceExit(typename OpenMdp<ValueType>::EntranceExit entranceExit,
                                                                                                        typename OpenMdp<ValueType>::Scope& scope) const {
    if (values.size() == 0) {
        STORM_LOG_ASSERT(false, "something went wrong");
        return {};
    }

    std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> entries;
    size_t i = 0;
    for (const auto& v : values) {
        scope.pushScope(i);
        auto newEntries = v->collectEntranceExit(entranceExit, scope);
        scope.popScope();
        entries.insert(std::end(entries), std::begin(newEntries), std::end(newEntries));
        ++i;
    }
    return entries;
}

template<typename ValueType>
std::vector<std::shared_ptr<OpenMdp<ValueType>>> SumModel<ValueType>::getValues() {
    return values;
}

template<typename ValueType>
bool SumModel<ValueType>::isRightward() const {
    for (const auto& value : values) {
        if (!value->isRightward()) {
            return false;
        }
    }
    return true;
}

template class SumModel<storm::RationalNumber>;
template class SumModel<double>;

}  // namespace models
}  // namespace storm
