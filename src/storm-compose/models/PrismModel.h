#pragma once

#include "OpenMdp.h"
#include "OpenMdpManager.h"

namespace storm {
namespace models {

template<typename ValueType>
class PrismModel : public OpenMdp<ValueType> {
    public:
    PrismModel(OpenMdpManager<ValueType>& manager, std::string path);

    bool isPrismModel() override;

    std::string getPath();

    private:
    std::string path;
};

}
}
