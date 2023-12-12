#pragma once

#include "AbstractCache.h"
#include "EntranceExit.h"
#include "storm/storage/geometry/Polytope.h"
#include "storm-compose/models/ConcreteMdp.h"

namespace storm {
namespace storage {

template<typename ValueType>
class ParetoCache : public AbstractCache<ValueType> {
   public:
    typedef std::vector<ValueType> WeightType;

    boost::optional<WeightType> getLowerBound(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight) override;
    void addToCache(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight, WeightType inputWeight, boost::optional<storm::storage::Scheduler<ValueType>> sched = boost::none) override;
    bool needScheduler() override;

   private:
    void initializeParetoCurve(models::ConcreteMdp<ValueType>* ptr);
    boost::optional<std::pair<WeightType, WeightType>> getLowerUpper(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight, Position pos);
    ValueType innerProduct(WeightType a, WeightType b);
    ValueType getError(WeightType lower, WeightType upper);
    void updateLowerUpperBounds(std::pair<models::ConcreteMdp<ValueType>*, Position> key, WeightType point, WeightType weight);
    bool isInitialized(models::ConcreteMdp<ValueType>* ptr);

    std::map<std::pair<models::ConcreteMdp<ValueType>*, Position>, std::shared_ptr<geometry::Polytope<ValueType>>> lowerBounds, upperBounds;
};

}  // namespace storage
}  // namespace storm
