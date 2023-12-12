#include "NoCache.h"

namespace storm {
namespace storage {

template<typename ValueType>
boost::optional<std::vector<ValueType>> NoCache<ValueType>::getLowerBound(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight) {
    return boost::none;
}

template<typename ValueType>
void NoCache<ValueType>::addToCache(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight, WeightType inputWeight, boost::optional<storm::storage::Scheduler<ValueType>> sched) {

}

template<typename ValueType>
bool NoCache<ValueType>::needScheduler() {
    return false;
}

template class NoCache<double>;
template class NoCache<storm::RationalNumber>;

}  // namespace storage
}  // namespace storm
