#pragma once

#include "OpenMdp.h"
#include "storm/models/sparse/Mdp.h"

namespace storm {
namespace models {

template<typename ValueType> class OpenMdp;

namespace visitor {
template<typename ValueType> class OpenMdpPrintVisitor;
template<typename ValueType> class OpenMdpToDotVisitor;
template<typename ValueType> class FlatMdpBuilderVisitor;
template<typename ValueType> class ParetoVisitor;
template<typename ValueType> class PropertyDrivenVisitor;
template<typename ValueType> class LowerUpperParetoVisitor;
}

template<typename ValueType>
class ConcreteMdp : public OpenMdp<ValueType> {
    // TODO remove these friend classes and introduce getter/setters instead
    friend class visitor::OpenMdpPrintVisitor<ValueType>;
    friend class visitor::OpenMdpToDotVisitor<ValueType>;
    friend class visitor::FlatMdpBuilderVisitor<ValueType>;
    friend class visitor::ParetoVisitor<ValueType>;
    friend class visitor::PropertyDrivenVisitor<ValueType>;
    friend class visitor::LowerUpperParetoVisitor<ValueType>;

public:
    ConcreteMdp(std::weak_ptr<OpenMdpManager<ValueType>> manager);
    ConcreteMdp(std::weak_ptr<OpenMdpManager<ValueType>> manager, std::shared_ptr<storm::models::sparse::Mdp<ValueType>> mdp,
                std::vector<size_t> lEntrance = {}, std::vector<size_t> rEntrance = {}, std::vector<size_t> lExit = {}, std::vector<size_t> rExit = {});
    //ConcreteMdp(ConcreteMdp const&) = default;
    ~ConcreteMdp();

    bool isConcreteMdp() const override;
    void accept(visitor::OpenMdpVisitor<ValueType>& visitor) override;
    std::shared_ptr<storm::models::sparse::Mdp<ValueType>> getMdp();
    const std::shared_ptr<storm::models::sparse::Mdp<ValueType>> getMdp() const;

    std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> collectEntranceExit(typename OpenMdp<ValueType>::EntranceExit entryExit, typename OpenMdp<ValueType>::Scope& scope) const override;
    void exportToDot(std::string path);

    bool isRightward() const override;
    std::vector<ValueType> weightedReachability(std::vector<ValueType> const& weights);
    void setWeights(std::vector<ValueType> const& weights);

private:
    std::shared_ptr<storm::models::sparse::Mdp<ValueType>> mdp;
    std::vector<size_t> lEntrance, rEntrance, lExit, rExit;
    bool isPreparedForWeightedReachability = false;
    void prepareForWeightedReachability();
};

}
}
