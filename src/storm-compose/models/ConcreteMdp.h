#pragma once

#include "OpenMdp.h"
#include "storm/models/sparse/Mdp.h"

namespace storm {
namespace models {

template<typename ValueType> class OpenMdp;

namespace visitor {
template<typename ValueType> class OpenMdpPrintVisitor;
template<typename ValueType> class OpenMdpToDotVisitor;
}

template<typename ValueType>
class ConcreteMdp : public OpenMdp<ValueType> {
    friend class visitor::OpenMdpPrintVisitor<ValueType>;
    friend class visitor::OpenMdpToDotVisitor<ValueType>;

    public:
    ConcreteMdp(OpenMdpManager<ValueType>& manager, std::shared_ptr<storm::models::sparse::Mdp<ValueType>> mdp,
                std::vector<size_t> lEntrance = {}, std::vector<size_t> rEntrance = {}, std::vector<size_t> lExit = {}, std::vector<size_t> rExit = {});
    ~ConcreteMdp();
    bool isConcreteMdp() const override;
    void accept(visitor::OpenMdpVisitor<ValueType>& visitor) override;
    std::shared_ptr<storm::models::sparse::Mdp<ValueType>> getMdp();
    const std::shared_ptr<storm::models::sparse::Mdp<ValueType>> getMdp() const;

    std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> collectEntranceExit(typename OpenMdp<ValueType>::EntranceExit entryExit, typename OpenMdp<ValueType>::Scope& scope) const override;

    private:
    std::shared_ptr<storm::models::sparse::Mdp<ValueType>> mdp;
    std::vector<size_t> lEntrance, rEntrance, lExit, rExit;
};

template class ConcreteMdp<double>;
template class ConcreteMdp<storm::RationalNumber>;

}
}
