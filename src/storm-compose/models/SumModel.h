#ifndef SUMMODEL_H
#define SUMMODEL_H

#include <vector>
#include "OpenMdp.h"
#include "OpenMdpManager.h"

namespace storm {
namespace models {

template<typename ValueType>
class SumModel : public OpenMdp<ValueType> {
   public:
    SumModel(std::shared_ptr<OpenMdpManager<ValueType>> manager, std::vector<std::shared_ptr<OpenMdp<ValueType>>> values);
    virtual void accept(visitor::OpenMdpVisitor<ValueType>& visitor) override;
    std::vector<std::shared_ptr<OpenMdp<ValueType>>>& getValues();

    bool isRightward() const override;

   private:
    std::vector<std::shared_ptr<OpenMdp<ValueType>>> values;
};

}  // namespace models
}  // namespace storm

#endif /* SUMMODEL_H */
