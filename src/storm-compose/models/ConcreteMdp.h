#pragma once

#include "OpenMdp.h"
#include "storm/models/sparse/Mdp.h"

namespace storm {
namespace models {

template<typename ValueType>
class ConcreteMdp : public OpenMdp<ValueType> {
    public:
    bool isConcreteMdp() override;

    private:
        storm::models::sparse::Mdp<ValueType> mdp;

};

}
}
