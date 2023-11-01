#pragma once

#include "OpenMdp.h"
#include "OpenMdpManager.h"
#include "ConcreteMdp.h"

namespace storm {
namespace models {

template<typename ValueType> class OpenMdp;
template<typename ValueType> class OpenMdpManager;
template<typename ValueType> class ConcreteMdp;

namespace visitor {
template<typename ValueType> class OpenMdpVisitor;
template<typename ValueType> class OpenMdpToDotVisitor;
template<typename ValueType> class ParetoVisitor;
template<typename ValueType> class LowerUpperParetoVisitor;
}

template<typename ValueType>
class PrismModel : public OpenMdp<ValueType> {
    friend class visitor::OpenMdpToDotVisitor<ValueType>;
    friend class visitor::ParetoVisitor<ValueType>;
    friend class visitor::LowerUpperParetoVisitor<ValueType>;

    public:
    PrismModel(std::weak_ptr<OpenMdpManager<ValueType>> manager, std::string path,
               std::vector<std::string> lEntrance = {}, std::vector<std::string> rEntrance = {},
               std::vector<std::string> lExit = {}, std::vector<std::string> rExit = {});
    ~PrismModel();

    bool isPrismModel() const override;
    std::string getPath() const;
    virtual void accept(visitor::OpenMdpVisitor<ValueType>& visitor) override;
    std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> collectEntranceExit(typename OpenMdp<ValueType>::EntranceExit entryExit, typename OpenMdp<ValueType>::Scope& scope) const override;
    ConcreteMdp<ValueType> toConcreteMdp();
    bool isRightward() const override;

    private:
    std::string path;
    std::vector<std::string> lEntrance, rEntrance, lExit, rExit;
};

}
}
