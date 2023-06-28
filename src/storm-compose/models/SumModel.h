#pragma once

#include "storm-compose/models/visitor/OpenMdpVisitor.h"
#include "OpenMdp.h"
#include "OpenMdpManager.h"
#include <vector>

namespace storm {
namespace models {

namespace visitor {
template <typename ValueType> class OpenMdpVisitor;
template <typename ValueType> class OpenMdpPrintVisitor;
template <typename ValueType> class OpenMdpToDotVisitor;
}

template <typename ValueType>
class SumModel : public OpenMdp<ValueType> {
    friend class visitor::OpenMdpPrintVisitor<ValueType>;
    friend class visitor::OpenMdpToDotVisitor<ValueType>;
    friend class visitor::OpenMdpVisitor<ValueType>;

    public:
    SumModel(OpenMdpManager<ValueType>& manager, std::vector<std::shared_ptr<OpenMdp<ValueType>>> values);
    virtual void accept(visitor::OpenMdpVisitor<ValueType>& visitor) override;
    std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> collectEntranceExit(typename OpenMdp<ValueType>::EntranceExit entryExit, typename OpenMdp<ValueType>::Scope& scope) const override;

    private:
    std::vector<std::shared_ptr<OpenMdp<ValueType>>> values;
};

}
}
