#pragma once

#include <vector>
#include "OpenMdp.h"
#include "OpenMdpManager.h"
#include "storm-compose/models/visitor/OpenMdpVisitor.h"

namespace storm {
namespace models {

namespace visitor {
template<typename ValueType>
class OpenMdpToDotVisitor;
template<typename ValueType>
class FlatMdpBuilderVisitor;
template<typename ValueType>
class ParetoVisitor;
template<typename ValueType>
class LowerUpperParetoVisitor;
}  // namespace visitor

template<typename ValueType>
class TraceModel : public OpenMdp<ValueType> {
    friend class visitor::OpenMdpToDotVisitor<ValueType>;
    friend class visitor::OpenMdpVisitor<ValueType>;
    friend class visitor::FlatMdpBuilderVisitor<ValueType>;
    friend class visitor::ParetoVisitor<ValueType>;
    friend class visitor::LowerUpperParetoVisitor<ValueType>;

   public:
    TraceModel(std::shared_ptr<OpenMdpManager<ValueType>> manager, std::shared_ptr<OpenMdp<ValueType>> value, size_t left, size_t right);
    virtual void accept(visitor::OpenMdpVisitor<ValueType>& visitor) override;
    std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> collectEntranceExit(typename OpenMdp<ValueType>::EntranceExit entryExit,
                                                                                       typename OpenMdp<ValueType>::Scope& scope) const override;
    std::shared_ptr<OpenMdp<ValueType>> getValue();

    bool isRightward() const override;
    size_t getLeft();
    size_t getRight();

   private:
    std::shared_ptr<OpenMdp<ValueType>> value;
    size_t left, right;
};

}  // namespace models
}  // namespace storm
