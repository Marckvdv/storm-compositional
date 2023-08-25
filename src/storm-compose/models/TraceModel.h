#pragma once

#include "storm-compose/models/visitor/OpenMdpVisitor.h"
#include "OpenMdp.h"
#include "OpenMdpManager.h"
#include <vector>

namespace storm {
namespace models {

namespace visitor {
template <typename ValueType> class OpenMdpToDotVisitor;
template <typename ValueType> class FlatMdpBuilderVisitor;
}

template <typename ValueType>
class TraceModel : public OpenMdp<ValueType> {
    friend class visitor::OpenMdpToDotVisitor<ValueType>;
    friend class visitor::OpenMdpVisitor<ValueType>;
    friend class visitor::FlatMdpBuilderVisitor<ValueType>;

    public:
    TraceModel(std::shared_ptr<OpenMdpManager<ValueType>> manager, std::shared_ptr<OpenMdp<ValueType>> value, size_t left, size_t right);
    virtual void accept(visitor::OpenMdpVisitor<ValueType>& visitor) override;
    std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> collectEntranceExit(typename OpenMdp<ValueType>::EntranceExit entryExit, typename OpenMdp<ValueType>::Scope& scope) const override;

    private:
    std::shared_ptr<OpenMdp<ValueType>> value;
    size_t left, right;
};

}
}
