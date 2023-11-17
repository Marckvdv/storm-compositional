#pragma once

#include "OpenMdp.h"
#include "storm-compose/models/visitor/OpenMdpVisitor.h"
#include "storm/models/sparse/Mdp.h"

namespace storm {
namespace models {

namespace visitor {
template<typename ValueType>
class OpenMdpVisitor;
}

template<typename ValueType>
class OpenMdp;
template<typename ValueType>
class OpenMdpManager;

template<typename ValueType>
class ConcreteMdp : public OpenMdp<ValueType> {
   public:
    ConcreteMdp(std::weak_ptr<OpenMdpManager<ValueType>> manager);
    ConcreteMdp(std::weak_ptr<OpenMdpManager<ValueType>> manager, std::shared_ptr<storm::models::sparse::Mdp<ValueType>> mdp,
                std::vector<size_t> lEntrance = {}, std::vector<size_t> rEntrance = {}, std::vector<size_t> lExit = {}, std::vector<size_t> rExit = {});
    // ConcreteMdp(ConcreteMdp const&) = default;
    ~ConcreteMdp();

    bool isConcreteMdp() const override;
    void accept(visitor::OpenMdpVisitor<ValueType>& visitor) override;
    std::shared_ptr<storm::models::sparse::Mdp<ValueType>> getMdp();
    const std::shared_ptr<storm::models::sparse::Mdp<ValueType>> getMdp() const;

    std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> collectEntranceExit(typename OpenMdp<ValueType>::EntranceExit entryExit,
                                                                                       typename OpenMdp<ValueType>::Scope& scope) const override;
    void exportToDot(std::string path);
    bool isRightward() const override;
    std::vector<ValueType> weightedReachability(std::vector<ValueType> const& weight);

    std::vector<size_t> const& getLEntrance() const;
    std::vector<size_t> const& getREntrance() const;
    std::vector<size_t> const& getLExit() const;
    std::vector<size_t> const& getRExit() const;

   private:
    std::shared_ptr<storm::models::sparse::Mdp<ValueType>> mdp;
    std::vector<size_t> lEntrance, rEntrance, lExit, rExit;
};

}  // namespace models
}  // namespace storm
