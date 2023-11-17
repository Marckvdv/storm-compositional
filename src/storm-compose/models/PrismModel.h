#pragma once

#include "ConcreteMdp.h"
#include "OpenMdp.h"
#include "OpenMdpManager.h"

namespace storm {
namespace models {

template<typename ValueType>
class OpenMdp;
template<typename ValueType>
class OpenMdpManager;
template<typename ValueType>
class ConcreteMdp;

template<typename ValueType>
class PrismModel : public OpenMdp<ValueType> {
   public:
    PrismModel(std::weak_ptr<OpenMdpManager<ValueType>> manager, std::string path, std::vector<std::string> lEntrance = {},
               std::vector<std::string> rEntrance = {}, std::vector<std::string> lExit = {}, std::vector<std::string> rExit = {});
    ~PrismModel();

    bool isPrismModel() const override;
    std::string getPath() const;
    virtual void accept(visitor::OpenMdpVisitor<ValueType>& visitor) override;
    std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> collectEntranceExit(typename OpenMdp<ValueType>::EntranceExit entryExit,
                                                                                       typename OpenMdp<ValueType>::Scope& scope) const override;
    ConcreteMdp<ValueType> toConcreteMdp();
    bool isRightward() const override;

    std::vector<std::string> const& getLEntrance() const;
    std::vector<std::string> const& getREntrance() const;
    std::vector<std::string> const& getLExit() const;
    std::vector<std::string> const& getRExit() const;

   private:
    std::string path;
    std::vector<std::string> lEntrance, rEntrance, lExit, rExit;
};

}  // namespace models
}  // namespace storm
