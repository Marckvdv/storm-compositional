#include "TrueParetoVisitor.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
TrueParetoVisitor<ValueType>::TrueParetoVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager) : ParetoVisitor<ValueType>(manager) {

}

template<typename ValueType>
void visitConcreteModel(ConcreteMdp<ValueType>& model) {
}

template<typename ValueType>
void TrueParetoVisitor<ValueType>::visitReference(Reference<ValueType>& reference) {
    //const auto& referenceName = reference.getReference();
    //auto it = paretoResults.find(referenceName);
    //if (it != paretoResults.end()) {
    //    // TODO avoid copying
    //    currentPareto = *it;
    //} else {
    //    const auto manager = reference.getManager();
    //    auto dereferenced = manager->dereference(referenceName);
    //    dereferenced->accept(*this); // after this currentPareto should be set
    //}
}

template<typename ValueType>
void TrueParetoVisitor<ValueType>::visitSequenceModel(SequenceModel<ValueType>& model) {
    ParetoVisitor<ValueType>::visitSequenceModel(model); // after this ConcreteMdp current is set.
}

template<typename ValueType>
void TrueParetoVisitor<ValueType>::visitSumModel(SumModel<ValueType>& model) {
    // TODO implement: call base class function and compute pareto results and store it.
}

template<typename ValueType>
void TrueParetoVisitor<ValueType>::visitTraceModel(TraceModel<ValueType>& model) {
    // TODO implement: Recurse over all values, after which
}

}
}
}
