#include "AbstractCache.h"
#include "storm-compose/models/ConcreteMdp.h"

#include <map>

namespace storm {
namespace storage {

template <typename ValueType>
class ExactCache : public AbstractCache<ValueType> {
   public:
    typedef std::vector<ValueType> WeightType;

    boost::optional<WeightType> getLowerBound(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight) override;
    void addToCache(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight, WeightType inputWeight, boost::optional<storm::storage::Scheduler<ValueType>> sched = boost::none) override;
    bool needScheduler() override;

   private:
    std::map<std::pair<WeightType, models::ConcreteMdp<ValueType>*>, WeightType> cache;
};

}  // namespace storage
}  // namespace storm
