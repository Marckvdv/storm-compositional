#pragma once

#include <boost/optional.hpp>
#include "storm/storage/Scheduler.h"
#include "storm-compose/models/OpenMdp.h"

namespace storm {
namespace storage {

template <typename ValueType>
class AbstractCache {
   public:
    typedef std::vector<ValueType> WeightType;

    virtual boost::optional<WeightType> getLowerBound(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight) = 0;
    virtual void addToCache(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight, WeightType inputWeight, boost::optional<storm::storage::Scheduler<ValueType>> sched = boost::none) = 0;

    virtual void setErrorTolerance(ValueType errorTolerance) {
        this->errorTolerance = errorTolerance;
    }

    virtual bool needScheduler() = 0;
   protected:
    ValueType errorTolerance;
};

}  // namespace storage
}  // namespace storm
