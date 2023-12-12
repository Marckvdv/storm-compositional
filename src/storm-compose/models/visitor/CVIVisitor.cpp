#include "CVIVisitor.h"

#include "storage/BitVector.h"
#include "storage/SparseMatrix.h"
#include "storm-compose/modelchecker/CompositionalValueIteration.h"
#include "storm-compose/models/visitor/ParetoVisitor.h"
#include "storm-parsers/parser/FormulaParser.h"
#include "storm/modelchecker/multiobjective/pcaa/StandardMdpPcaaWeightVectorChecker.h"
#include "storm/modelchecker/multiobjective/preprocessing/SparseMultiObjectivePreprocessor.h"
#include "storm-compose/models/visitor/EntranceExitVisitor.h"
#include "storm/environment/Environment.h"
#include "storm/environment/solver/SolverEnvironment.h"
#include "utility/constants.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
CVIVisitor<ValueType>::CVIVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager, ValueVector<ValueType>& valueVector, std::shared_ptr<storm::storage::AbstractCache<ValueType>> cache) : manager(manager), valueVector(valueVector), cache(cache) {
}

template<typename ValueType>
CVIVisitor<ValueType>::~CVIVisitor() {
}

template<typename ValueType>
void CVIVisitor<ValueType>::visitPrismModel(PrismModel<ValueType>& model) {
    STORM_LOG_ASSERT(false, "Expected concrete models");
}

template<typename ValueType>
void CVIVisitor<ValueType>::visitConcreteModel(ConcreteMdp<ValueType>& model) {
    std::vector<ValueType> weights;
    bool allZero = true;
    for (size_t i = 0; i < model.getLExit().size(); ++i) {
        std::pair<storage::EntranceExit, size_t> pos {storage::L_EXIT, currentLeftExitPosition};
        ValueType weight = valueVector.getWeight(currentLeafId, pos);
        if (weight != 0) allZero = false;
        weights.push_back(weight);

        ++currentLeftExitPosition;
    }
    for (size_t i = 0; i < model.getRExit().size(); ++i) {
        std::pair<storage::EntranceExit, size_t> pos {storage::R_EXIT, currentRightExitPosition};
        ValueType weight = valueVector.getWeight(currentLeafId, pos);
        if (weight != 0) allZero = false;
        weights.push_back(weight);

        ++currentRightExitPosition;
    }

    std::vector<ValueType> inputValues;
    if (allZero) {
        inputValues = std::vector<ValueType>(model.getLEntrance().size() + model.getREntrance().size(), 0);
    } else {
        boost::optional<std::vector<ValueType>> result;
        result = cache->getLowerBound(&model, weights);

        if (result) {
            inputValues = *result;
        } else {
            auto newResult = weightedReachability(weights, model, cache->needScheduler(), env);
            auto weight = newResult.first;
            auto scheduler = newResult.second;

            inputValues = weight;
            cache->addToCache(&model, weights, inputValues, scheduler);
        }
    }
    //std::cout << "size of input values " << inputValues.size() << std::endl;
    size_t weightIndex = 0;
    for (size_t i = 0; i < model.getLEntrance().size(); ++i) {
        std::pair<storage::EntranceExit, size_t> pos {storage::L_ENTRANCE, currentLeftPosition};
        valueVector.setWeight(currentLeafId, pos, inputValues[weightIndex]);

        ++weightIndex;
        ++currentLeftPosition;
    }

    for (size_t i = 0; i < model.getREntrance().size(); ++i) {
        std::pair<storage::EntranceExit, size_t> pos {storage::R_ENTRANCE, currentRightPosition};
        valueVector.setWeight(currentLeafId, pos, inputValues[weightIndex]);

        ++weightIndex;
        ++currentRightPosition;
    }

    ++currentLeafId;
}

template<typename ValueType>
void CVIVisitor<ValueType>::visitSequenceModel(SequenceModel<ValueType>& model) {
    currentSequencePosition = 0;

    auto& values = model.getValues();
    for (const auto& v : values) {
        currentLeftPosition = 0;
        currentRightPosition = 0;
        currentLeftExitPosition = 0;
        currentRightExitPosition = 0;

        //currentScope.pushScope(currentSequencePosition);
        v->accept(*this);
        //currentScope.popScope();
        ++currentSequencePosition;
    }
}

template<typename ValueType>
void CVIVisitor<ValueType>::visitSumModel(SumModel<ValueType>& model) {
    auto& values = model.getValues();
    for (const auto& v : values) {
        v->accept(*this);
    }
}

template<typename ValueType>
std::pair<std::vector<ValueType>, boost::optional<storm::storage::Scheduler<ValueType>>> CVIVisitor<ValueType>::weightedReachability(std::vector<ValueType> weights, ConcreteMdp<ValueType> concreteMdp, bool returnScheduler, storm::Environment env) {
    using storm::modelchecker::multiobjective::StandardMdpPcaaWeightVectorChecker;
    using storm::modelchecker::multiobjective::preprocessing::SparseMultiObjectivePreprocessor;
    using storm::models::sparse::Mdp;

    std::string formulaString = ParetoVisitor<ValueType>::getFormula(concreteMdp);

    storm::parser::FormulaParser formulaParser;
    auto formula = formulaParser.parseSingleFormulaFromString(formulaString);
    auto mdp = concreteMdp.getMdp();

    std::vector<ValueType> newWeights;

    boost::optional<storm::storage::Scheduler<ValueType>> scheduler;
    auto computeReachability = [&](const auto& entrances) {
        for (size_t entrance : entrances) {
            // TODO make efficient
            mdp->getStateLabeling().setStates("init", storage::BitVector(mdp->getNumberOfStates()));
            mdp->getStateLabeling().addLabelToState("init", entrance);

            //env.solver().setLinearEquationSolverPrecision(storm::RationalNumber(1e-6));
            auto preprocessResult = SparseMultiObjectivePreprocessor<Mdp<ValueType>>::preprocess(env, *mdp, formula->asMultiObjectiveFormula());
            StandardMdpPcaaWeightVectorChecker checker(preprocessResult);

            checker.check(env, weights);
            auto underApprox = checker.getUnderApproximationOfInitialStateResults();

            if (returnScheduler && !scheduler) {
                scheduler = checker.computeScheduler();
            }

            ValueType sum = 0;
            for (size_t i = 0; i < weights.size(); ++i) {
                sum += weights[i] * underApprox[i];
                //std::cout << underApprox[i] << std::endl;
            }

            newWeights.push_back(sum);
        }
    };
    computeReachability(concreteMdp.getLEntrance());
    computeReachability(concreteMdp.getREntrance());

    return {newWeights, scheduler};
}

template<typename ValueType>
std::vector<ValueType> CVIVisitor<ValueType>::weightedReachability2(std::vector<ValueType> weights, ConcreteMdp<ValueType> concreteMdp) {
    using storm::modelchecker::multiobjective::StandardMdpPcaaWeightVectorChecker;
    using storm::modelchecker::multiobjective::preprocessing::SparseMultiObjectivePreprocessor;
    using storm::models::sparse::Mdp;

    std::string formulaString = ParetoVisitor<ValueType>::getFormula(concreteMdp);

    storm::parser::FormulaParser formulaParser;
    auto formula = formulaParser.parseSingleFormulaFromString(formulaString);
    auto mdp = concreteMdp.getMdp();
    auto transitionMatrix = mdp->getTransitionMatrix();

    std::vector<ValueType> newWeights;

    auto rewardModels = mdp->getRewardModels();

    std::map<size_t, ValueType> stateToWeightMap; // TODO populate
    storm::storage::SparseMatrixBuilder<ValueType> rewardMatrixBuilder(0, 0, 0, true, true);

    for (size_t rowGroup = 0; rowGroup < transitionMatrix.getRowGroupCount(); ++rowGroup) {
        for (size_t action = 0; action < transitionMatrix.getRowGroupSize(rowGroup); ++action) {
            auto row = transitionMatrix.getRow(rowGroup, action);

            for (const auto entry : row) {
                auto column = entry.getColumn();
                const auto it = stateToWeightMap.find(column);
                if (it != stateToWeightMap.end()) {
                    //rewardMatrixBuilder.addNextValue(index_type row, index_type column, const value_type &value)
                } else {

                }
            }
        }
    }

    return newWeights;
}

template class CVIVisitor<storm::RationalNumber>;
template class CVIVisitor<double>;

}  // namespace visitor
}  // namespace models
}  // namespace storm
