#include "CVIVisitor.h"
#include <vector>

#include "exceptions/IllegalArgumentValueException.h"
#include "modelchecker/prctl/helper/SparseMdpPrctlHelper.h"
#include "models/sparse/StandardRewardModel.h"
#include "storage/BitVector.h"
#include "storage/SparseMatrix.h"
#include "storm-compose/benchmark/BenchmarkStats.h"
#include "storm-compose/modelchecker/CompositionalValueIteration.h"
#include "storm-compose/models/visitor/EntranceExitVisitor.h"
#include "storm-compose/models/visitor/ParetoVisitor.h"
#include "storm-compose/storage/EntranceExit.h"
#include "storm-parsers/parser/FormulaParser.h"
#include "storm/environment/Environment.h"
#include "storm/environment/solver/SolverEnvironment.h"
#include "storm/exceptions/NotSupportedException.h"
#include "storm/modelchecker/multiobjective/pcaa/StandardMdpPcaaWeightVectorChecker.h"
#include "storm/modelchecker/multiobjective/preprocessing/SparseMultiObjectivePreprocessor.h"
#include "storm/models/sparse/StandardRewardModel.h"
#include "storm/utility/constants.h"
#include "utility/vector.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
CVIVisitor<ValueType>::CVIVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager, ValueVector<ValueType>& valueVector,
                                  std::shared_ptr<storm::storage::AbstractCache<ValueType>> cache, compose::benchmark::BenchmarkStats<ValueType>& stats)
    : manager(manager), valueVector(valueVector), cache(cache), stats(stats) {}

template<typename ValueType>
CVIVisitor<ValueType>::~CVIVisitor() {}

template<typename ValueType>
void CVIVisitor<ValueType>::visitPrismModel(PrismModel<ValueType>& model) {
    STORM_LOG_ASSERT(false, "Expected concrete models");
}

template<typename ValueType>
void CVIVisitor<ValueType>::visitConcreteModel(ConcreteMdp<ValueType>& model) {
    std::vector<ValueType> weights;
    bool allZero = true;
    for (size_t i = 0; i < model.getLExit().size(); ++i) {
        std::pair<storage::EntranceExit, size_t> pos{storage::L_EXIT, currentLeftExitPosition};
        ValueType weight = valueVector.getWeight(currentLeafId, pos);
        if (weight != 0)
            allZero = false;
        weights.push_back(weight);

        ++currentLeftExitPosition;
    }
    for (size_t i = 0; i < model.getRExit().size(); ++i) {
        std::pair<storage::EntranceExit, size_t> pos{storage::R_EXIT, currentRightExitPosition};
        ValueType weight = valueVector.getWeight(currentLeafId, pos);
        if (weight != 0)
            allZero = false;
        weights.push_back(weight);

        ++currentRightExitPosition;
    }

    std::vector<ValueType> inputValues;
    if (allZero) {
        inputValues = std::vector<ValueType>(model.getLEntrance().size() + model.getREntrance().size(), 0);
    } else {
        boost::optional<std::vector<ValueType>> result;
        result = queryCache(&model, weights);

        ++stats.weightedReachabilityQueries;
        if (result) {
            inputValues = *result;
            ++stats.cacheHits;
        } else {
            stats.reachabilityComputationTime.start();
            auto newResult = weightedReachability2(weights, model, cache->needScheduler(), env);
            // auto newResult = weightedReachability(weights, model, cache->needScheduler(), env);
            stats.reachabilityComputationTime.stop();
            auto weight = newResult.first;
            auto scheduler = newResult.second;

            inputValues = weight;
            addToCache(&model, weights, inputValues, scheduler);
        }
    }

    size_t weightIndex = 0;
    for (size_t i = 0; i < model.getLEntrance().size(); ++i) {
        std::pair<storage::EntranceExit, size_t> pos{storage::L_ENTRANCE, currentLeftPosition};
        valueVector.setWeight(currentLeafId, pos, inputValues[weightIndex]);

        ++weightIndex;
        ++currentLeftPosition;
    }

    for (size_t i = 0; i < model.getREntrance().size(); ++i) {
        std::pair<storage::EntranceExit, size_t> pos{storage::R_ENTRANCE, currentRightPosition};
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

        v->accept(*this);
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
std::pair<std::vector<ValueType>, boost::optional<storm::storage::Scheduler<ValueType>>> CVIVisitor<ValueType>::weightedReachability(
    std::vector<ValueType> weights, ConcreteMdp<ValueType> concreteMdp, bool returnScheduler, storm::Environment env) {
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

            // env.solver().setLinearEquationSolverPrecision(storm::RationalNumber(1e-6));
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
            }

            newWeights.push_back(sum);
        }
    };
    computeReachability(concreteMdp.getLEntrance());
    computeReachability(concreteMdp.getREntrance());

    return {newWeights, scheduler};
}

template<typename ValueType>
std::pair<std::vector<ValueType>, boost::optional<storm::storage::Scheduler<ValueType>>> CVIVisitor<ValueType>::weightedReachability2(
    std::vector<ValueType> weights, ConcreteMdp<ValueType> concreteMdp, bool returnScheduler, storm::Environment env) {
    static size_t EXACT_QUERIES = 0;
    std::cout << "Exact queries: " << EXACT_QUERIES << " Mdp size: " << concreteMdp.getMdp()->getTransitionMatrix().getRowGroupCount() << std::endl;
    ++EXACT_QUERIES;

    if (false) {
        std::ofstream f("out.dot");
        concreteMdp.getMdp()->writeDotToStream(f);
        f.close();
    }

    using storm::models::sparse::Mdp;

    std::string formulaString = ParetoVisitor<ValueType>::getFormula(concreteMdp);

    storm::parser::FormulaParser formulaParser;
    auto formula = formulaParser.parseSingleFormulaFromString(formulaString);
    auto mdp = concreteMdp.getMdp();
    auto transitionMatrix = mdp->getTransitionMatrix();

    std::map<size_t, storm::storage::Position> stateToPositionMap;
    storm::storage::BitVector initialStates(transitionMatrix.getRowGroupCount());
    size_t positionIndex = 0;
    for (const auto& state : concreteMdp.getLEntrance()) {
        initialStates.set(state);
        stateToPositionMap[state] = {storage::L_ENTRANCE, positionIndex};

        ++positionIndex;
    }
    positionIndex = 0;
    for (const auto& state : concreteMdp.getREntrance()) {
        initialStates.set(state);
        stateToPositionMap[state] = {storage::R_ENTRANCE, positionIndex};

        ++positionIndex;
    }
    mdp->setInitialStates(initialStates);

    size_t weightIndex = 0;
    std::map<size_t, ValueType> stateToWeightMap;
    storm::storage::BitVector exitStates(transitionMatrix.getRowGroupCount());
    for (const auto& state : concreteMdp.getLExit()) {
        // std::cout << "state: " << state << " -> " << weights[weightIndex] << std::endl;
        stateToWeightMap[state] = weights[weightIndex];
        exitStates.set(state);

        ++weightIndex;
    }
    for (const auto& state : concreteMdp.getRExit()) {
        // std::cout << "state: " << state << " -> " << weights[weightIndex] << std::endl;
        stateToWeightMap[state] = weights[weightIndex];
        exitStates.set(state);

        ++weightIndex;
    }

    storm::storage::SparseMatrixBuilder<ValueType> rewardMatrixBuilder(transitionMatrix.getRowCount(), transitionMatrix.getColumnCount(), 0, true, true,
                                                                       transitionMatrix.getRowGroupCount());

    for (size_t rowGroup = 0; rowGroup < transitionMatrix.getRowGroupCount(); ++rowGroup) {
        size_t rowGroupStartIndex = transitionMatrix.getRowGroupIndices()[rowGroup];
        rewardMatrixBuilder.newRowGroup(rowGroupStartIndex);
        if (exitStates[rowGroup])
            continue;

        for (size_t action = 0; action < transitionMatrix.getRowGroupSize(rowGroup); ++action) {
            auto row = transitionMatrix.getRow(rowGroup, action);

            for (const auto entry : row) {
                auto column = entry.getColumn();
                const auto it = stateToWeightMap.find(column);
                if (it != stateToWeightMap.end()) {
                    rewardMatrixBuilder.addNextValue(rowGroupStartIndex + action, column, it->second);
                }
            }
        }
    }

    auto rewardMatrix = rewardMatrixBuilder.build();
    // std::cout << "Reward Matrix: " << std::endl << rewardMatrix << std::endl;
    storm::models::sparse::StandardRewardModel<ValueType> rewardModel(std::nullopt, std::nullopt, rewardMatrix);
    STORM_LOG_ASSERT(transitionMatrix.isProbabilistic(), "not probabilistic");

    auto result = modelchecker::helper::SparseMdpPrctlHelper<ValueType>::computeTotalRewards(
        env, storm::solver::SolveGoal<ValueType>(false), transitionMatrix, mdp->getBackwardTransitions(), rewardModel, false, returnScheduler);
    boost::optional<storm::storage::Scheduler<ValueType>> scheduler;
    if (returnScheduler) {
        scheduler = *result.scheduler;  // TODO prevent copy
    }

    std::vector<ValueType> newWeights(concreteMdp.getEntranceCount());

    for (size_t state : initialStates) {
        auto pos = stateToPositionMap.at(state);
        size_t index = pos.second;
        if (pos.first == storm::storage::R_ENTRANCE) {
            index += concreteMdp.getLEntrance().size();
        }
        newWeights[index] = result.values[state];
    }

    return {newWeights, scheduler};
}

template<typename ValueType>
void CVIVisitor<ValueType>::visitTraceModel(TraceModel<ValueType>& model) {
    STORM_LOG_THROW(false, storm::exceptions::NotSupportedException, "Trace operator currently not supported");
}

template<typename ValueType>
boost::optional<typename CVIVisitor<ValueType>::WeightType> CVIVisitor<ValueType>::queryCache(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight) {
    stats.cacheRetrievalTime.start();
    auto result = cache->getLowerBound(ptr, outputWeight);
    stats.cacheRetrievalTime.stop();

    return result;
}

template<typename ValueType>
void CVIVisitor<ValueType>::addToCache(models::ConcreteMdp<ValueType>* ptr, WeightType outputWeight, WeightType inputWeight,
                                       boost::optional<storm::storage::Scheduler<ValueType>> sched) {
    stats.cacheInsertionTime.start();
    cache->addToCache(ptr, outputWeight, inputWeight, sched);
    stats.cacheInsertionTime.stop();
}

template class CVIVisitor<storm::RationalNumber>;
template class CVIVisitor<double>;

}  // namespace visitor
}  // namespace models
}  // namespace storm
