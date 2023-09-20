#pragma once

#include "OpenMdp.h"

namespace storm {
namespace models {

template<typename ValueType> class OpenMdp;
template<typename ValueType> class OpenMdpManager;

namespace visitor {
template<typename ValueType> class OpenMdpVisitor;
}

template<typename ValueType>
class Reference : public OpenMdp<ValueType> {
    public:
    Reference(std::shared_ptr<OpenMdpManager<ValueType>> manager, std::string reference);
    ~Reference() override {}

    bool isReference() const override;

    std::string getReference() const;
    virtual void accept(visitor::OpenMdpVisitor<ValueType>& visitor) override;
    std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> collectEntranceExit(typename OpenMdp<ValueType>::EntranceExit entryExit, typename OpenMdp<ValueType>::Scope& scope) const override;

    bool isRightward() const override;

    private:
    std::string reference;
};

}
}
