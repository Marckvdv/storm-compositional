#include "LeafTransformer.h"
#include "exceptions/UnexpectedException.h"
#include "storm-compose/models/OpenMdpManager.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
LeafTransformer<ValueType>::LeafTransformer(std::function<ConcreteMdp<ValueType>(ConcreteMdp<ValueType>&)> transformer)
    : transformer(transformer), manager(std::make_shared<OpenMdpManager<ValueType>>()) {}

template<typename ValueType>
void LeafTransformer<ValueType>::visitPrismModel(PrismModel<ValueType>& model) {
    STORM_LOG_THROW(false, storm::exceptions::UnexpectedException, "Expected concrete MDPs");
}

template<typename ValueType>
void LeafTransformer<ValueType>::visitConcreteModel(ConcreteMdp<ValueType>& model) {
    result = std::make_shared<ConcreteMdp<ValueType>>(transformer(model));
}

template<typename ValueType>
void LeafTransformer<ValueType>::visitReference(Reference<ValueType>& reference) {
    const auto oldManager = reference.getManager();
    auto dereferenced = oldManager->dereference(reference.getReference());
    dereferenced->accept(*this);
    manager->setReference(reference.getReference(), result);
    result = std::make_shared<Reference<ValueType>>(this->manager, reference.getReference());
}

template<typename ValueType>
void LeafTransformer<ValueType>::visitSequenceModel(SequenceModel<ValueType>& model) {
    std::vector<std::shared_ptr<OpenMdp<ValueType>>> values;
    for (auto& value : model.getValues()) {
        value->accept(*this);
        values.push_back(result);
    }
    result = std::make_shared<SequenceModel<ValueType>>(this->manager, values);
}

template<typename ValueType>
void LeafTransformer<ValueType>::visitSumModel(SumModel<ValueType>& model) {
    std::vector<std::shared_ptr<OpenMdp<ValueType>>> values;
    for (auto& value : model.getValues()) {
        value->accept(*this);
        values.push_back(result);
    }
    result = std::make_shared<SumModel<ValueType>>(this->manager, values);
}

template<typename ValueType>
void LeafTransformer<ValueType>::visitTraceModel(TraceModel<ValueType>& model) {
    model.accept(*this);
    result = std::make_shared<TraceModel<ValueType>>(this->manager, result, model.getLeft(), model.getRight());
}

template<typename ValueType>
std::shared_ptr<OpenMdp<ValueType>> LeafTransformer<ValueType>::getResult() {
    return result;
}

template<typename ValueType>
std::shared_ptr<OpenMdpManager<ValueType>> LeafTransformer<ValueType>::getManager() {
    manager->setRoot(result);
    return manager;
}

template class LeafTransformer<double>;
template class LeafTransformer<storm::RationalNumber>;

}  // namespace visitor
}  // namespace models
}  // namespace storm
