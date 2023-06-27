#include "FlatMdpBuilder.h"
#include "storm/models/sparse/StateLabeling.h"

namespace storm {
namespace builder {

template <typename ValueType>
FlatMdpBuilder<ValueType>::FlatMdpBuilder(OpenMdp<ValueType> const& openMdp) : openMdp(openMdp) {
}


template <typename ValueType>
Mdp<ValueType> FlatMdpBuilder<ValueType>::build() {
    auto matrix = buildMatrix();

    storm::models::sparse::StateLabeling labeling;
    Mdp<ValueType> mdp(matrix, labeling);

    return mdp;
}

template <typename ValueType>
SparseMatrix<ValueType> FlatMdpBuilder<ValueType>::buildMatrix() {
    /*
     * How to build the complete matrix of the flat MDP?  It is clear that we
     * need a copy of the transition matrix for each concrete MDP.  Perhaps we
     * can construct the full transition matrix by recursing over the open MDP,
     * This will not be the most efficient approach, though that is not really
     * important at the moment.
     *
     * This would take an Open MDP and turn it in to a concrete MDP.  For a
     * concrete MDP this does nothing.  For a Sequence, we compute the
     * underlying concrete MDPs and string together their entrances and exits.
     * For a Sum, we put the concrete MDPs in parallel and combine their
     * entrances and exists.
     * For a Trace, we simply connect exits to entrances.
     *
     */
}

}
}
