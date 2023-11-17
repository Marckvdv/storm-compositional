#include "BenchmarkStatsVisitor.h"

#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/models/SequenceModel.h"
#include "storm-compose/models/SumModel.h"
#include "storm-compose/models/TraceModel.h"

namespace storm {
namespace models {
namespace visitor {

template<class ValueType>
BenchmarkStatsVisitor<ValueType>::BenchmarkStatsVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager,
                                                        storm::compose::benchmark::BenchmarkStats<ValueType>& stats)
    : manager(manager), stats(stats) {
    // intentionally left empty
}

template<class ValueType>
void BenchmarkStatsVisitor<ValueType>::visitPrismModel(PrismModel<ValueType>& model) {
    STORM_LOG_ASSERT(false, "concretize first before calling");
}

template<class ValueType>
void BenchmarkStatsVisitor<ValueType>::visitConcreteModel(ConcreteMdp<ValueType>& model) {
    increaseDepth();

    size_t stateCount = model.getMdp()->getNumberOfStates();
    stats.stateCount += stateCount;

    if (visitingUniqueLeave) {
        stats.leafStates += stateCount;

        const auto& transitionMatrix = model.getMdp()->getTransitionMatrix();

        // Overapproximate the number of schedulers in the given model
        size_t maxActionCount = transitionMatrix.getSizeOfLargestRowGroup();
        storm::RationalNumber product = storm::utility::pow(storm::RationalNumber(maxActionCount), transitionMatrix.getRowGroupCount());
        stats.leafSchedulerCount += product;
    }

    decreaseDepth();
}

template<class ValueType>
void BenchmarkStatsVisitor<ValueType>::visitReference(Reference<ValueType>& reference) {
    // increaseDepth();

    std::string referenceName = reference.getReference();
    auto openMdp = reference.getManager()->dereference(referenceName);

    if (openMdp->isConcreteMdp()) {
        if (uniqueLeaves.find(referenceName) == uniqueLeaves.end()) {
            stats.uniqueLeaves += 1;
            uniqueLeaves.insert(referenceName);
            visitingUniqueLeave = true;
        }
    }

    openMdp->accept(*this);
    visitingUniqueLeave = false;

    // decreaseDepth();
}

template<class ValueType>
void BenchmarkStatsVisitor<ValueType>::visitSequenceModel(SequenceModel<ValueType>& model) {
    increaseDepth();

    stats.sequenceCount += model.getValues().size();
    for (const auto& v : model.getValues()) {
        v->accept(*this);
    }

    decreaseDepth();
}

template<class ValueType>
void BenchmarkStatsVisitor<ValueType>::visitSumModel(SumModel<ValueType>& model) {
    increaseDepth();

    stats.sumCount += model.getValues().size();
    for (const auto& v : model.getValues()) {
        v->accept(*this);
    }

    decreaseDepth();
}

template<class ValueType>
void BenchmarkStatsVisitor<ValueType>::visitTraceModel(TraceModel<ValueType>& model) {
    increaseDepth();

    stats.traceCount += model.getLeft() + model.getRight();
    model.getValue()->accept(*this);

    decreaseDepth();
}

template<class ValueType>
void BenchmarkStatsVisitor<ValueType>::increaseDepth() {
    ++depth;
    if (depth > stats.stringDiagramDepth) {
        stats.stringDiagramDepth = depth;
    }
}

template<class ValueType>
void BenchmarkStatsVisitor<ValueType>::decreaseDepth() {
    --depth;
    STORM_LOG_ASSERT(depth >= 0, "sanity check");
}

template class BenchmarkStatsVisitor<double>;
template class BenchmarkStatsVisitor<storm::RationalNumber>;

}  // namespace visitor
}  // namespace models
}  // namespace storm