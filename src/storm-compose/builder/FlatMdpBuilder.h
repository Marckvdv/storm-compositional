#pragma once

#include "storm-compose/models/OpenMdp.h"
#include "storm/storage/SparseMatrix.h"

namespace storm {
namespace builder {

using storm::models::OpenMdp;
using storm::storage::SparseMatrixBuilder;
using storm::storage::SparseMatrix;
using storm::models::sparse::Mdp;

template <typename ValueType>
class FlatMdpBuilder {
public:
    FlatMdpBuilder(OpenMdp<ValueType> const& openMdp);
    Mdp<ValueType> build();

private:
    SparseMatrix<ValueType> buildMatrix();

    OpenMdp<ValueType> const& openMdp;
    SparseMatrixBuilder<ValueType> matrixBuilder;
};

}
}
