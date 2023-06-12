#pragma once

#include "OpenMdp.h"
#include "OpenMdpManager.h"
#include <vector>

namespace storm {
namespace models {

template <typename ValueType>
class SumModel : public OpenMdp<ValueType> {
    public:
    SumModel(OpenMdpManager<ValueType>& manager, std::vector<std::shared_ptr<OpenMdp<ValueType>>> values);

    private:
    std::vector<std::shared_ptr<OpenMdp<ValueType>>> values;
};

}
}
