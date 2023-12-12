#include "FlatMdpBuilderVisitor.h"
#include "storm-compose/models/SequenceModel.h"
#include "storm-compose/models/SumModel.h"
#include "storm-compose/models/TraceModel.h"

namespace storm {
namespace models {
namespace visitor {

using storm::models::OpenMdp;
using storm::models::sparse::Mdp;
using storm::storage::SparseMatrix;
using storm::storage::SparseMatrixBuilder;

template<typename ValueType>
FlatMdpBuilderVisitor<ValueType>::FlatMdpBuilderVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager) : manager(manager), current(manager) {}

template<typename ValueType>
void FlatMdpBuilderVisitor<ValueType>::visitPrismModel(PrismModel<ValueType>& model) {
    STORM_LOG_ASSERT(false, "Expected all Prism models to be concrete!");
}

template<typename ValueType>
void FlatMdpBuilderVisitor<ValueType>::visitConcreteModel(ConcreteMdp<ValueType>& model) {
    // Model already concrete so we do not need to do anything.
    // ConcreteMdp<ValueType> test = model;
    current = model;
}

template<typename ValueType>
void FlatMdpBuilderVisitor<ValueType>::visitSequenceModel(SequenceModel<ValueType>& model) {
    // Obtain concrete MDPs
    size_t totalStateCount = 0;
    std::vector<ConcreteMdp<ValueType>> concreteMdps;
    for (auto& m : model.getValues()) {
        m->accept(*this);
        concreteMdps.push_back(current);  // TODO remove redundant copying
        totalStateCount += current.getMdp()->getNumberOfStates();
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
    storm::storage::SparseMatrixBuilder<ValueType> builder(0, 0, 0, true, true);
    storm::models::sparse::StateLabeling labeling(totalStateCount);
    labeling.addLabel("init");
    size_t offset = 0;
    std::vector<size_t> lEntrance, lExit, rEntrance, rExit;

    size_t currentRow = 0;
    size_t currentEntrance = 0;
    size_t currentExit = 0;

    for (size_t v : concreteMdps[0].getLEntrance()) {
        lEntrance.push_back(v);
        std::string entranceLabel = "len" + std::to_string(currentEntrance);
        labeling.addLabel(entranceLabel);
        labeling.addLabelToState(entranceLabel, v);
        ++currentEntrance;
    }
    for (size_t v : concreteMdps[0].getLExit()) {
        lExit.push_back(v);
        std::string exitLabel = "lex" + std::to_string(currentExit);
        labeling.addLabel(exitLabel);
        labeling.addLabelToState(exitLabel, v);
        ++currentExit;
    }

    for (size_t i = 0; i < concreteMdps.size(); ++i) {
        const auto& c = concreteMdps[i];
        const auto& transitionMatrix = c.getMdp()->getTransitionMatrix();
        // Iterate over all states
        size_t stateCount = transitionMatrix.getRowGroupCount();
        for (size_t state = 0; state < stateCount; ++state) {
            builder.newRowGroup(currentRow);

            auto lExitPos = std::find(c.getLExit().begin(), c.getLExit().end(), state);
            auto rExitPos = std::find(c.getRExit().begin(), c.getRExit().end(), state);
            // TODO do the above using a map
            // TODO cannot be both lExit and rExit

            if (i != 0 && lExitPos != c.getLExit().end()) {
                // State is a left exit
                const auto& previous = concreteMdps[i - 1];
                size_t exit = lExitPos - c.getLExit().begin();
                size_t previousOffset = offset - previous.getMdp()->getNumberOfStates();
                size_t entranceState = previousOffset + previous.getREntrance()[exit];

                builder.addNextValue(currentRow++, entranceState, 1);
            } else if (i != concreteMdps.size() - 1 && rExitPos != c.getRExit().end()) {
                // State is a right exit
                const auto& next = concreteMdps[i + 1];
                size_t exit = rExitPos - c.getRExit().begin();
                size_t nextOffset = offset + c.getMdp()->getNumberOfStates();
                size_t entranceState = nextOffset + next.getLEntrance()[exit];

                builder.addNextValue(currentRow++, entranceState, 1);
            } else {
                // State is not an exit

                // Iterate over all actions
                size_t actionCount = transitionMatrix.getRowGroupSize(state);
                for (size_t action = 0; action < actionCount; ++action) {
                    const auto& row = transitionMatrix.getRow(state, action);

                    // Iterate over all entries
                    for (const auto& entry : row) {
                        // And add it to the new transition matrix
                        builder.addNextValue(currentRow, offset + entry.getColumn(), entry.getValue());
                    }
                    ++currentRow;
                }
            }
        }

        offset += stateCount;
    }

    size_t lastIndex = concreteMdps.size() - 1;
    size_t lastOffset = offset - concreteMdps[lastIndex].getMdp()->getNumberOfStates();
    currentEntrance = 0;
    for (size_t v : concreteMdps[lastIndex].getREntrance()) {
        rEntrance.push_back(lastOffset + v);
        std::string entranceLabel = "ren" + std::to_string(currentEntrance);
        labeling.addLabel(entranceLabel);
        labeling.addLabelToState(entranceLabel, lastOffset + v);
        ++currentEntrance;
    }
    currentExit = 0;
    for (size_t v : concreteMdps[lastIndex].getRExit()) {
        rExit.push_back(lastOffset + v);
        std::string exitLabel = "rex" + std::to_string(currentExit);
        labeling.addLabel(exitLabel);
        labeling.addLabelToState(exitLabel, lastOffset + v);
        ++currentExit;
    }

    auto newMdp = std::make_shared<Mdp<ValueType>>(builder.build(), labeling);
    current = ConcreteMdp<ValueType>(manager, newMdp, lEntrance, rEntrance, lExit, rExit);
}

template<typename ValueType>
void FlatMdpBuilderVisitor<ValueType>::visitSumModel(SumModel<ValueType>& model) {
    // Obtain concrete MDPs
    std::vector<ConcreteMdp<ValueType>> concreteMdps;
    size_t totalStateCount = 0;
    for (auto& m : model.getValues()) {
        m->accept(*this);
        concreteMdps.push_back(current);  // TODO remove redundant copying
        totalStateCount += current.getMdp()->getNumberOfStates();
    }

    // Stitch them together.
    storm::storage::SparseMatrixBuilder<ValueType> builder(0, 0, 0, true, true);
    storm::models::sparse::StateLabeling labeling(totalStateCount);

    size_t offset = 0;
    std::vector<size_t> lEntrance, lExit, rEntrance, rExit;

    size_t leftExitCount = 0, rightExitCount = 0, leftEntranceCount = 0, rightEntranceCount = 0;
    size_t currentRow = 0;
    for (const auto& c : concreteMdps) {
        const auto& transitionMatrix = c.getMdp()->getTransitionMatrix();
        // Iterate over all states
        size_t stateCount = transitionMatrix.getRowGroupCount();
        for (size_t state = 0; state < stateCount; ++state) {
            builder.newRowGroup(currentRow);

            // Iterate over all actions
            size_t actionCount = transitionMatrix.getRowGroupSize(state);
            for (size_t action = 0; action < actionCount; ++action) {
                const auto& row = transitionMatrix.getRow(state, action);

                // Iterate over all entries
                for (const auto& entry : row) {
                    // And add it to the new transition matrix
                    builder.addNextValue(currentRow, offset + entry.getColumn(), entry.getValue());
                }

                ++currentRow;
            }
        }

        for (size_t v : c.getLEntrance()) {
            lEntrance.push_back(offset + v);

            std::string label = "len" + std::to_string(leftEntranceCount);
            labeling.addLabel(label);
            labeling.addLabelToState(label, offset + v);
            ++leftEntranceCount;
        }

        for (size_t v : c.getREntrance()) {
            rEntrance.push_back(offset + v);

            std::string label = "ren" + std::to_string(rightEntranceCount);
            labeling.addLabel(label);
            labeling.addLabelToState(label, offset + v);
            ++rightEntranceCount;
        }

        for (size_t v : c.getLExit()) {
            lExit.push_back(offset + v);

            std::string label = "lex" + std::to_string(leftExitCount);
            labeling.addLabel(label);
            labeling.addLabelToState(label, offset + v);
            ++leftExitCount;
        }

        for (size_t v : c.getRExit()) {
            rExit.push_back(offset + v);

            std::string label = "rex" + std::to_string(rightExitCount);
            labeling.addLabel(label);
            labeling.addLabelToState(label, offset + v);
            ++rightExitCount;
        }

        offset += stateCount;
    }

    auto newMdp = std::make_shared<Mdp<ValueType>>(builder.build(), labeling);
    current = ConcreteMdp<ValueType>(manager, newMdp, lEntrance, rEntrance, lExit, rExit);
}

template<typename ValueType>
void FlatMdpBuilderVisitor<ValueType>::visitTraceModel(TraceModel<ValueType>& model) {
    // Obtain concrete MDP
    model.value->accept(*this);

    // We ignore transitions that are present in the exits.  We simply add
    // an unique action that goes to the corresponding entrance with
    // probability 0.

    storm::storage::SparseMatrixBuilder<ValueType> builder(0, 0, 0, true, true);
    const size_t totalStateCount = current.getMdp()->getNumberOfStates();
    storm::models::sparse::StateLabeling labeling(totalStateCount);
    labeling.addLabel("init");
    size_t currentRow = 0;

    const auto& transitionMatrix = current.getMdp()->getTransitionMatrix();
    // Iterate over all states
    size_t stateCount = transitionMatrix.getRowGroupCount();
    for (size_t state = 0; state < stateCount; ++state) {
        builder.newRowGroup(currentRow);

        auto lExitPos = std::find(current.getLExit().begin(), current.getLExit().end(), state);
        auto rExitPos = std::find(current.getRExit().begin(), current.getRExit().end(), state);
        // TODO do the above using a map
        // TODO cannot be both lExit and rExit

        bool isExit = false;
        if (lExitPos != current.getLExit().end()) {
            isExit = true;

            // State is a left exit
            size_t exit = lExitPos - current.getLExit().begin();
            if (exit >= model.left) {
                builder.addNextValue(currentRow++, state, 1);
            } else {
                size_t entranceState = current.getREntrance()[exit];
                builder.addNextValue(currentRow++, entranceState, 1);
            }
        } else if (rExitPos != current.getRExit().end()) {
            isExit = true;

            // State is a right exit
            size_t exit = rExitPos - current.getRExit().begin();
            if (exit >= model.right) {
                builder.addNextValue(currentRow++, state, 1);
            } else {
                size_t entranceState = current.getLEntrance()[exit];
                builder.addNextValue(currentRow++, entranceState, 1);
            }
        }
        if (isExit)
            continue;
        // State is not an exit

        // Iterate over all actions
        size_t actionCount = transitionMatrix.getRowGroupSize(state);
        for (size_t action = 0; action < actionCount; ++action) {
            const auto& row = transitionMatrix.getRow(state, action);

            // Iterate over all entries
            for (const auto& entry : row) {
                // And add it to the new transition matrix
                builder.addNextValue(currentRow, entry.getColumn(), entry.getValue());
            }
            ++currentRow;
        }
    }

    for (size_t i = builder.getCurrentRowGroupCount(); i < totalStateCount; ++i) {
        builder.newRowGroup(currentRow);
        builder.addNextValue(currentRow, i, 1);
        ++currentRow;
    }

    size_t currentEntrance = 0;
    std::vector<size_t> lEntrance, lExit, rEntrance, rExit;
    for (size_t i = model.left; i < current.getREntrance().size(); ++i) {
        rEntrance.push_back(current.getREntrance()[i]);

        std::string label = "ren" + std::to_string(currentEntrance);
        labeling.addLabel(label);
        labeling.addLabelToState(label, current.getREntrance()[i]);
        ++currentEntrance;
    }

    currentEntrance = 0;
    for (size_t i = model.right; i < current.getLEntrance().size(); ++i) {
        lEntrance.push_back(current.getLEntrance()[i]);

        std::string label = "len" + std::to_string(currentEntrance);
        labeling.addLabel(label);
        labeling.addLabelToState(label, current.getLEntrance()[i]);
        ++currentEntrance;
    }

    size_t currentExit = 0;
    for (size_t i = model.left; i < current.getLExit().size(); ++i) {
        lExit.push_back(current.getLExit()[i]);

        std::string label = "lex" + std::to_string(currentExit);
        labeling.addLabel(label);
        labeling.addLabelToState(label, current.getLExit()[i]);
        ++currentExit;
    }

    currentExit = 0;
    for (size_t i = model.right; i < current.getRExit().size(); ++i) {
        rExit.push_back(current.getRExit()[i]);

        std::string label = "rex" + std::to_string(currentExit);
        labeling.addLabel(label);
        labeling.addLabelToState(label, current.getRExit()[i]);
        ++currentExit;
    }

    auto newMdp = std::make_shared<Mdp<ValueType>>(builder.build(), labeling);
    current = ConcreteMdp<ValueType>(manager, newMdp, lEntrance, rEntrance, lExit, rExit);
}

template<typename ValueType>
ConcreteMdp<ValueType> FlatMdpBuilderVisitor<ValueType>::getCurrent() {
    return current;
}

template class FlatMdpBuilderVisitor<storm::RationalNumber>;
template class FlatMdpBuilderVisitor<double>;

}  // namespace visitor
}  // namespace models
}  // namespace storm
