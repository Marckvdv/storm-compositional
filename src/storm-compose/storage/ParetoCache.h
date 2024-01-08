#pragma once

#include "AbstractCache.h"
#include "EntranceExit.h"
#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/models/OpenMdpManager.h"
#include "storm-compose/models/visitor/BidirectionalReachabilityResult.h"
#include "storm/storage/geometry/Polytope.h"

namespace storm {
namespace storage {

template<typename ValueType>
class ParetoCache : public AbstractCache<ValueType> {
   public:
    typedef std::vector<ValueType> WeightType;
    typedef storm::RationalNumber ParetoRational;
    typedef std::vector<ParetoRational> ParetoPointType;
    typedef std::vector<ParetoPointType> LowerBoundType;
    typedef std::shared_ptr<storm::storage::geometry::Polytope<storm::RationalNumber>> UpperBoundType;
    typedef std::pair<models::ConcreteMdp<ValueType>*, Position> KeyType;

    boost::optional<WeightType> getLowerBound(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight) override;
    void addToCache(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight, WeightType inputWeight,
                    boost::optional<storm::storage::Scheduler<ValueType>> sched = boost::none) override;
    bool needScheduler() override;

    std::shared_ptr<storm::models::ConcreteMdp<ValueType>> toLowerBoundShortcutMdp(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager,
                                                                                   storm::models::ConcreteMdp<ValueType>* model);
    std::shared_ptr<storm::models::ConcreteMdp<ValueType>> toUpperBoundShortcutMdp(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager,
                                                                                   storm::models::ConcreteMdp<ValueType>* model);

   private:
    void initializeParetoCurve(models::ConcreteMdp<ValueType>* ptr);
    std::pair<ParetoPointType, ParetoPointType> getLowerUpper(models::ConcreteMdp<ValueType>* ptr, ParetoPointType outputWeight, Position pos);

    // computes the inf-norm of upper-lower
    ParetoRational getError(ParetoPointType lower, ParetoPointType upper) const;
    void updateLowerUpperBounds(KeyType key, ParetoPointType point, ParetoPointType weight);
    bool isInitialized(models::ConcreteMdp<ValueType>* ptr) const;

    storm::models::visitor::BidirectionalReachabilityResult<ValueType> getLowerBoundReachabilityResult(storm::models::ConcreteMdp<ValueType>* model);
    storm::models::visitor::BidirectionalReachabilityResult<ValueType> getUpperBoundReachabilityResult(storm::models::ConcreteMdp<ValueType>* model);

    std::map<KeyType, LowerBoundType> lowerBounds;
    std::map<KeyType, UpperBoundType> upperBounds;
};

}  // namespace storage
}  // namespace storm
