#include "ParetoVisitor.h"

#include "storm/api/storm.h"
#include "storm-parsers/api/storm-parsers.h"
#include "storage/prism/Program.h"
#include "storm/logic/Formula.h"
#include "storm/storage/jani/Property.h"
#include "storm/modelchecker/results/ExplicitParetoCurveCheckResult.h"
#include "storm/environment/Environment.h"
#include "storm/environment/modelchecker/MultiObjectiveModelCheckerEnvironment.h"
#include "storm/modelchecker/multiobjective/multiObjectiveModelChecking.h"

#include <memory>

namespace storm {
namespace models {
namespace visitor {

using storm::models::OpenMdp;
using storm::storage::SparseMatrixBuilder;
using storm::storage::SparseMatrix;
using storm::models::sparse::Mdp;

template<typename ValueType>
ParetoVisitor<ValueType>::ParetoVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager) : manager(manager) {
}

template<typename ValueType>
void ParetoVisitor<ValueType>::visitPrismModel(PrismModel<ValueType>& model) {
    auto program = storm::api::parseProgram(model.getPath(), false, false);

    std::string formulaString = getFormula(model);
    std::vector<std::shared_ptr<storm::logic::Formula const>> formulas =
        storm::api::extractFormulasFromProperties(storm::api::parsePropertiesForPrismProgram(formulaString, program));
    STORM_LOG_ASSERT(formulas.size() == 1, "sanity check");
    auto formula = formulas.at(0);
    std::cout << "Parsed formula: " << *formula << std::endl;

    storm::Environment env;
    env.modelchecker().multi().setMethod(storm::modelchecker::multiobjective::MultiObjectiveMethod::Pcaa);

    const auto& manager = program.getManager();
    storm::parser::ExpressionParser parser(manager);
    parser.setIdentifierMapping(getIdentifierMapping(manager));

    BidirectionalReachabilityResult<ValueType> results;
    //std::vector<std::unique_ptr<storm::modelchecker::CheckResult>> leftParetoResults, rightParetoResults;
    auto checkEntrances = [&](const auto& entrances, bool leftEntrance) {
        // TODO find away we do not have to rebuild the MDP each time.
        // This should be possible by setting multiple initial states during generation,
        // and then selecting only a single initial state during model checking.
        size_t entranceNumber = 0;
        for (auto const& entrance : entrances) {
            auto initialStatesExpression = parser.parseFromString(entrance);
            std::cout << initialStatesExpression << std::endl;

            storm::prism::InitialConstruct initialConstruct(initialStatesExpression);
            program.setInitialConstruct(initialConstruct);

            std::shared_ptr<storm::models::sparse::Mdp<ValueType>> mdp =
                storm::api::buildSparseModel<ValueType>(program, formulas)->template as<storm::models::sparse::Mdp<ValueType>>();
            std::unique_ptr<storm::modelchecker::CheckResult> result =
                storm::modelchecker::multiobjective::performMultiObjectiveModelChecking(env, *mdp, formula->asMultiObjectiveFormula());

            if (result->isExplicitParetoCurveCheckResult()) {
                auto paretoResult = result->template asExplicitParetoCurveCheckResult<ValueType>();
                const auto& points = paretoResult.getPoints();

                for (size_t j = 0; j < points.size(); ++j) {
                    const auto& point = points[j];
                    results.addPoint(entranceNumber, leftEntrance, point);
                }
            } else {
                STORM_LOG_ASSERT(false, "TODO");
                /*
                STORM_LOG_ASSERT(paretoResults[i]->isExplicitQuantitativeCheckResult(), "result was not pareto nor quantitative");
                STORM_LOG_ASSERT(model.lExit.size() + model.rExit.size() == 1, "Expected only 1 exit");
                auto quantitativeResult = paretoResults[i]->template asExplicitQuantitativeCheckResult<ValueType>();

                // TODO below we assume that the initial state is 0, but this may not be the case (?)
                builder.addNextValue(currentRow, exitOffset, quantitativeResult[0]);
                ++currentRow;
                */
            }

            entranceNumber++;
            //paretoResults.push_back(std::move(result));
        }
    };

    checkEntrances(model.lEntrance, true);
    checkEntrances(model.rEntrance, false);

    /*
    const size_t exitOffset = leftParetoResults.size() + rightParetoResults.size();
    size_t currentRow = 0;
    storm::storage::SparseMatrixBuilder<ValueType> builder(0, 0, 0, true, true);

    auto buildTransitions = [&](const auto& paretoResults) {
        for (size_t i = 0; i < paretoResults.size(); ++i) {
            builder.newRowGroup(currentRow);

            if (paretoResults[i]->isExplicitParetoCurveCheckResult()) {
                auto paretoResult = paretoResults[i]->template asExplicitParetoCurveCheckResult<ValueType>();
                const auto& points = paretoResult.getPoints();

                for (size_t j = 0; j < points.size(); ++j) {
                    const auto& point = points[j];
                    for (size_t k = 0; k < point.size(); ++k) {
                        builder.addNextValue(currentRow, exitOffset + k, point[k]);
                    }
                    ++currentRow;
                }
            } else {
                STORM_LOG_ASSERT(paretoResults[i]->isExplicitQuantitativeCheckResult(), "result was not pareto nor quantitative");
                STORM_LOG_ASSERT(model.lExit.size() + model.rExit.size() == 1, "Expected only 1 exit");
                auto quantitativeResult = paretoResults[i]->template asExplicitQuantitativeCheckResult<ValueType>();

                // TODO below we assume that the initial state is 0, but this may not be the case (?)
                builder.addNextValue(currentRow, exitOffset, quantitativeResult[0]);
                ++currentRow;
            }
        }
    };

    buildTransitions(leftParetoResults);
    buildTransitions(rightParetoResults);

    const size_t totalStateCount = model.lEntrance.size() + model.lExit.size() + model.rEntrance.size() + model.rExit.size();
    for (size_t i = builder.getCurrentRowGroupCount(); i < totalStateCount; ++i) {
        builder.newRowGroup(currentRow);
        builder.addNextValue(currentRow, i, 1);
        ++currentRow;
    }

    std::vector<size_t> lEntrance, rEntrance, lExit, rExit;
    size_t currentState = 0;

    auto pushEntrancesExits = [&] (auto &entranceExit, const auto& source) {
        for (size_t i = 0; i < source.size(); ++i, ++currentState) {
            entranceExit.push_back(currentState);
        }
    };
    pushEntrancesExits(lEntrance, model.lEntrance);
    pushEntrancesExits(rEntrance, model.rEntrance);
    pushEntrancesExits(lExit, model.lExit);
    pushEntrancesExits(rExit, model.rExit);

    auto matrix = builder.build();
    storm::models::sparse::StateLabeling labeling(matrix.getRowGroupCount());
    auto newMdp = std::make_shared<Mdp<ValueType>>(matrix, labeling);
    */

    // TODO store pareto result
    //this->current = ConcreteMdp<ValueType>(this->manager, newMdp, lEntrance, rEntrance, lExit, rExit);
}

template<typename ValueType>
void ParetoVisitor<ValueType>::visitConcreteModel(ConcreteMdp<ValueType>& model) {
    STORM_LOG_ASSERT(false, "Expect prism models");
    // TODO somehow implement this. Should be mostly the same as the function above.
    // Need to call:
    //
    // storm::modelchecker::multiobjective::performMultiObjectiveModelChecking(env, *mdp, formula->asMultiObjectiveFormula());
    //
    // somehow, but without access to some Prism program. The mdp we have, but not the formula.
    // If we can construct the formula manually, referring to states in stead of valuations, we should be good.
    // Alternatively, we could add labels to the states and use those instead.
}

template<typename ValueType>
void ParetoVisitor<ValueType>::visitReference(Reference<ValueType>& reference) {
    const auto& referenceName = reference.getReference();
    auto it = paretoResults.find(referenceName);
    if (it != paretoResults.end()) {
        // TODO avoid copying
        currentPareto = it->second;
    } else {
        const auto manager = reference.getManager();
        auto dereferenced = manager->dereference(referenceName);
        dereferenced->accept(*this); // after this currentPareto should be set

        paretoResults[referenceName] = currentPareto;
    }
}

template<typename ValueType>
void ParetoVisitor<ValueType>::visitSequenceModel(SequenceModel<ValueType>& model) {
    // TODO implement
    // 1) Get Pareto result for each child
    // 2) Turn each Pareto result into a MDP
    // 3) Stitch them together, using code in the FlatMDP builder.

    // 1)
    std::vector<std::shared_ptr<OpenMdp<ValueType>>> concreteMdps;
    for (const auto& openMdp : model.getValues()) {
        openMdp->accept(*this);

        // 2)
        std::shared_ptr<ConcreteMdp<ValueType>> concreteMdp = currentPareto.toConcreteMdp();
        concreteMdps.push_back(std::static_pointer_cast<OpenMdp<ValueType>>(concreteMdp));
    }

    SequenceModel<ValueType> newSequenceModel(manager, concreteMdps);
    FlatMdpBuilderVisitor<ValueType> flatBuilder(manager);
    newSequenceModel.accept(flatBuilder);

    ConcreteMdp<ValueType> stitchedMdp = flatBuilder.getCurrent();

}

template<typename ValueType>
void ParetoVisitor<ValueType>::visitSumModel(SumModel<ValueType>& model) {
    // TODO implement
    // 1) Get Pareto result for each child
    // 2) Turn each Pareto result into a MDP
    // 3) Stitch them together, using code in the FlatMDP builder.
}

template<typename ValueType>
void ParetoVisitor<ValueType>::visitTraceModel(TraceModel<ValueType>& model) {
    // TODO implement
    // 1) Get Pareto result for the child
    // 2) Turn each Pareto result into a MDP
    // 3) Stitch them together, using code in the FlatMDP builder.
}

template<typename ValueType>
std::string ParetoVisitor<ValueType>::getFormula(PrismModel<ValueType>& model, bool rewards) {
    std::stringstream formulaBuffer;
    formulaBuffer << "multi(\n";

    bool first = true;
    auto reachTarget = [&](const auto& l) {
        for (const auto& v : l) {
            if (!first) {
                formulaBuffer << ", ";
            } else {
                formulaBuffer << "  ";
            }

            formulaBuffer << "Pmax=? [F (" << v << ")]";
            if (rewards) {
                formulaBuffer << ", Rmax=? [F (" << v << ")]";
            }
            first = false;
            formulaBuffer << "\n";
        }
    };
    reachTarget(model.lExit);
    reachTarget(model.rExit);

    formulaBuffer << ")";

    return formulaBuffer.str();
}

template<typename ValueType>
std::unordered_map<std::string, storm::expressions::Expression> ParetoVisitor<ValueType>::getIdentifierMapping(storm::expressions::ExpressionManager const& manager) {
    std::unordered_map<std::string, storm::expressions::Expression> result;
    for (const auto& var : manager.getVariables()) {
        result[var.getName()] = var.getExpression();
    }
    return result;
}

template class ParetoVisitor<double>;
template class ParetoVisitor<storm::RationalNumber>;

}
}
}
