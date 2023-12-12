#pragma once

#include <vector>
#include "OpenMdp.h"
#include "OpenMdpManager.h"
#include "storm-compose/models/visitor/OpenMdpVisitor.h"

namespace storm {
namespace models {

template<typename ValueType>
class OpenMdpManager;

template<typename ValueType>
class SequenceModel : public OpenMdp<ValueType> {
   public:
    SequenceModel(std::shared_ptr<OpenMdpManager<ValueType>> manager, std::vector<std::shared_ptr<OpenMdp<ValueType>>> values);

    virtual void accept(visitor::OpenMdpVisitor<ValueType>& visitor) override;
    std::vector<std::shared_ptr<OpenMdp<ValueType>>>& getValues();

    bool isRightward() const override;

   private:
    std::vector<std::shared_ptr<OpenMdp<ValueType>>> values;
};

}  // namespace models
}  // namespace storm
