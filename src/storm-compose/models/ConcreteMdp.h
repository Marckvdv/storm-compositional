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

    void exportToDot(std::string path);
    bool isRightward() const override;

    std::vector<size_t> const& getLEntrance() const;
    std::vector<size_t> const& getREntrance() const;
    std::vector<size_t> const& getLExit() const;
    std::vector<size_t> const& getRExit() const;
    size_t getEntranceCount() const;
    size_t getExitCount() const;

   private:
    std::shared_ptr<storm::models::sparse::Mdp<ValueType>> mdp;
    std::vector<size_t> lEntrance, rEntrance, lExit, rExit;
};

}  // namespace models
}  // namespace storm
