#pragma once

#include "storm-compose/models/OpenMdp.h"
#include "storm/storage/SparseMatrix.h"
#include "OpenMdpVisitor.h"

namespace storm {
namespace models {
namespace visitor {

using storm::models::OpenMdp;
using storm::storage::SparseMatrixBuilder;
using storm::storage::SparseMatrix;
using storm::models::sparse::Mdp;

template <typename ValueType>
class FlatMdpBuilderVisitor : public OpenMdpVisitor<ValueType> {
public:
    FlatMdpBuilderVisitor(OpenMdpManager<ValueType>& manager);
    Mdp<ValueType> build();

    virtual void visitPrismModel(PrismModel<ValueType>& model) override {
        STORM_LOG_ASSERT(false, "Expected all Prism models to be concrete!");
    }

    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override {
        // Model already concrete so we do not need to do anything.
        current = model;
    }

    // We use the default implementation which simply dereferences and visits that
    //virtual void visitReference(Reference<ValueType>& reference) override {
    //}

    virtual void visitSequenceModel(SequenceModel<ValueType>& model) override {
        // Obtain concrete MDPs
        std::vector<ConcreteMdp<ValueType>> concreteMdps;
        for (auto& m : model.values) {
            m->accept(*this);
            concreteMdps.push_back(current); // TODO remove redundant copying
        }

        // Stitch them together.
        // This is done as follows:
        // 1) Copy over the first matrix and connect the right exits to the left
        //    entrances of the second matrix. Also connect left exists of the
        //    second matrix to the right entrances of the first matrix.
        // 2) Update the offset value, which keeps track of where in the matrix
        //    we are. The offset is incremented by the state count of the first
        //    matrix.
        // 3) Repeat with all consecutive pairs of MDPs.
        // 4) Afterwards, the left entrances and exists are exactly that of the
        //    first MDP. The right entrances and exists are exactly that of the
        //    last MDP + offset.
        storm::storage::SparseMatrixBuilder<ValueType> builder;
    }

    virtual void visitSumModel(SumModel<ValueType>& model) override {
        // Obtain concrete MDPs
        std::vector<ConcreteMdp<ValueType>> concreteMdps;
        for (auto& m : model.values) {
            m->accept(*this);
            concreteMdps.push_back(current); // TODO remove redundant copying
        }

        // Stitch them together.
    }

    virtual void visitTraceModel(TraceModel<ValueType>& model) override {
        // Obtain concrete MDP
        model.value->accept(*this);
        auto concrete = current;
    }

private:
    OpenMdpManager<ValueType> const& manager;
    ConcreteMdp<ValueType> current;
};

}
}
}
