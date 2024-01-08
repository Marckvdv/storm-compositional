#include "HeuristicValueIterator.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
HeuristicValueIterator<ValueType>::HeuristicValueIterator(std::shared_ptr<models::OpenMdpManager<ValueType>> manager,
                                                          models::visitor::ValueVector<ValueType>& valueVector,
                                                          std::shared_ptr<storage::AbstractCache<ValueType>> cache,
                                                          compose::benchmark::BenchmarkStats<ValueType>& stats)
    : manager(manager), valueVector(valueVector), cache(cache), stats(stats) {
    // Intentionally left empty
}

template<typename ValueType>
void HeuristicValueIterator<ValueType>::performIteration() {}

template<typename ValueType>
void HeuristicValueIterator<ValueType>::updateModel(models::ConcreteMdp<ValueType>* model) {
    auto& valueVectorMapping = valueVector.getMapping();
    size_t leafId = valueVectorMapping.getLeafId(model);


}

template class HeuristicValueIterator<double>;
template class HeuristicValueIterator<storm::RationalNumber>;

}  // namespace modelchecker
}  // namespace storm
