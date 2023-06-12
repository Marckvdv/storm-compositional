#pragma once

#include "OpenMdp.h"
#include "OpenMdpManager.h"
#include <vector>

namespace storm {
namespace models {

template <typename ValueType>
class SequenceModel : public OpenMdp<ValueType> {
    public:
    SequenceModel(OpenMdpManager<ValueType>& manager, std::vector<std::shared_ptr<OpenMdp<ValueType>>> values);

    private:
    std::vector<std::shared_ptr<OpenMdp<ValueType>>> values;
};

}
}
