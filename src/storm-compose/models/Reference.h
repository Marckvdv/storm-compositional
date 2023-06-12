#pragma once

#include "OpenMdp.h"

namespace storm {
namespace models {

template<typename ValueType>
class Reference : public OpenMdp<ValueType> {
    public:
    Reference(OpenMdpManager<ValueType>& manager, std::string reference);
    ~Reference() override {}

    bool isReference() override;

    std::string getReference();

    private:
    std::string reference;
};

}
}
