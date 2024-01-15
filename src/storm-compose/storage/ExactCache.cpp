#include "ExactCache.h"
#include "storm-compose/models/ConcreteMdp.h"
#include "storm/adapters/RationalNumberAdapter.h"
#include "utility/constants.h"

namespace storm {
namespace storage {

template<typename ValueType>
ExactCache<ValueType>::ExactCache(ValueType oviEpsilon) : oviEpsilon(oviEpsilon) {}

template<typename ValueType>
boost::optional<std::vector<ValueType>> ExactCache<ValueType>::getLowerBound(models::ConcreteMdp<ValueType>* ptr, std::vector<ValueType> outputWeight) {
    const auto it = cache.find({outputWeight, ptr});
    if (it != cache.end()) {
        return it->second;
    } else {
        return boost::none;
    }
}

template<typename ValueType>
boost::optional<std::vector<ValueType>> ExactCache<ValueType>::getUpperBound(models::ConcreteMdp<ValueType>* ptr, std::vector<ValueType> outputWeight) {
    const auto it = cache.find({outputWeight, ptr});
    if (it != cache.end()) {
        auto values = it->second;
        for (auto& v : values) {
            v = storm::utility::min<ValueType>(v + oviEpsilon, storm::utility::one<ValueType>());
        }
        return values;
    } else {
        return boost::none;
    }
}

template<typename ValueType>
void ExactCache<ValueType>::addToCache(models::ConcreteMdp<ValueType>* ptr, std::vector<ValueType> outputWeight, std::vector<ValueType> inputWeight,
                                       boost::optional<storm::storage::Scheduler<ValueType>> sched) {
    cache.insert({{outputWeight, ptr}, inputWeight});
}

template<typename ValueType>
bool ExactCache<ValueType>::needScheduler() {
    return false;
}

template class ExactCache<double>;
template class ExactCache<storm::RationalNumber>;

}  // namespace storage
}  // namespace storm
