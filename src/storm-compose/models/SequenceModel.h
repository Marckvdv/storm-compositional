#pragma once

#include "storm-compose/models/visitor/OpenMdpVisitor.h"
#include "storm-compose/models/visitor/FlatMdpBuilderVisitor.h"
#include "OpenMdp.h"
#include "OpenMdpManager.h"
#include <vector>

namespace storm {
namespace models {

namespace visitor {
template <typename ValueType> class OpenMdpVisitor;
template <typename ValueType> class OpenMdpToDotVisitor;
template <typename ValueType> class OpenMdpPrintVisitor;
template <typename ValueType> class FlatMdpBuilderVisitor;
}

template <typename ValueType>
class SequenceModel : public OpenMdp<ValueType> {
    friend class visitor::OpenMdpToDotVisitor<ValueType>;
    friend class visitor::OpenMdpPrintVisitor<ValueType>;
    friend class visitor::OpenMdpVisitor<ValueType>;
    friend class visitor::FlatMdpBuilderVisitor<ValueType>;

    public:
    SequenceModel(std::shared_ptr<OpenMdpManager<ValueType>> manager, std::vector<std::shared_ptr<OpenMdp<ValueType>>> values);

    virtual void accept(visitor::OpenMdpVisitor<ValueType>& visitor) override;
    std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> collectEntranceExit(typename OpenMdp<ValueType>::EntranceExit entryExit, typename OpenMdp<ValueType>::Scope& scope) const override;
    std::vector<std::shared_ptr<OpenMdp<ValueType>>>& getValues();

    private:
    std::vector<std::shared_ptr<OpenMdp<ValueType>>> values;
};

}
}
