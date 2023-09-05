#include "ParetoVisitor.h"

#include "storm/api/storm.h"
#include "storm-parsers/api/storm-parsers.h"
#include "storage/prism/Program.h"
#include "storm/logic/Formula.h"
#include "storm/storage/jani/Property.h"
#include "storm/modelchecker/results/ExplicitParetoCurveCheckResult.h"
#include "storm/environment/modelchecker/MultiObjectiveModelCheckerEnvironment.h"
#include "storm/modelchecker/multiobjective/multiObjectiveModelChecking.h"
#include "storm-parsers/parser/FormulaParser.h"

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
    env.modelchecker().multi().setMethod(storm::modelchecker::multiobjective::MultiObjectiveMethod::Pcaa);
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

    const auto& manager = program.getManager();
    storm::parser::ExpressionParser parser(manager);
    parser.setIdentifierMapping(getIdentifierMapping(manager));

    BidirectionalReachabilityResult<ValueType> results(model.lEntrance.size(), model.rEntrance.size(), model.lExit.size(), model.rExit.size());
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
                storm::modelchecker::multiobjective::performMultiObjectiveModelChecking(this->env, *mdp, formula->asMultiObjectiveFormula());

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
        }
    };

    checkEntrances(model.lEntrance, true);
    checkEntrances(model.rEntrance, false);

    currentPareto = results;
}

template<typename ValueType>
void ParetoVisitor<ValueType>::visitConcreteModel(ConcreteMdp<ValueType>& model) {
    std::string formulaString = getFormula(model);
    storm::parser::FormulaParser formulaParser;
    auto formula = formulaParser.parseSingleFormulaFromString(formulaString);
    //std::vector<std::shared_ptr<storm::logic::Formula const>> formulas =
    //    storm::api::extractFormulasFromProperties(storm::api::parsePropertiesForPrismProgram(formulaString, program));
    std::cout << "Formula: " << *formula << std::endl;
    std::cout << "Initial states 2: " << model.getMdp()->getStateLabeling() << std::endl;
    std::cout << "Initial states: " << model.getMdp()->getInitialStates() << std::endl;

    BidirectionalReachabilityResult<ValueType> results(model.lEntrance.size(), model.rEntrance.size(), model.lExit.size(), model.rExit.size());
    size_t entranceNumber = 0;

    auto checkEntrances = [&](const auto& entrances, bool leftEntrance) {
        for (auto const& entrance : entrances) {
            // TODO set initial state
            
            model.getMdp()->getStateLabeling().addLabelToState("init", entranceNumber);
            std::unique_ptr<storm::modelchecker::CheckResult> result =
                storm::modelchecker::multiobjective::performMultiObjectiveModelChecking(this->env, *model.getMdp(), formula->asMultiObjectiveFormula());
            model.getMdp()->getStateLabeling().removeLabelFromState("init", entranceNumber);

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
        }
    };

    checkEntrances(model.lEntrance, true);
    checkEntrances(model.rEntrance, false);
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
    // 1) Get Pareto result for each child
    // 2) Turn each Pareto result into a MDP
    // 3) Stitch them together, using code in the FlatMDP builder.

    // 1)
    std::vector<std::shared_ptr<OpenMdp<ValueType>>> concreteMdps;
    for (const auto& openMdp : model.getValues()) {
        openMdp->accept(*this);

        // 2)
        std::shared_ptr<ConcreteMdp<ValueType>> concreteMdp = currentPareto.toConcreteMdp(manager);
        concreteMdps.push_back(std::static_pointer_cast<OpenMdp<ValueType>>(concreteMdp));
    }

    // 3)
    SequenceModel<ValueType> newSequenceModel(manager, concreteMdps);
    FlatMdpBuilderVisitor<ValueType> flatBuilder(manager);
    newSequenceModel.accept(flatBuilder);

    ConcreteMdp<ValueType> stitchedMdp = flatBuilder.getCurrent();
    visitConcreteModel(stitchedMdp);
}

template<typename ValueType>
void ParetoVisitor<ValueType>::visitSumModel(SumModel<ValueType>& model) {
    // 1) Get Pareto result for each child
    // 2) Turn each Pareto result into a MDP
    // 3) Stitch them together, using code in the FlatMDP builder.

    // 1)
    std::vector<std::shared_ptr<OpenMdp<ValueType>>> concreteMdps;
    for (const auto& openMdp : model.getValues()) {
        openMdp->accept(*this);

        // 2)
        std::shared_ptr<ConcreteMdp<ValueType>> concreteMdp = currentPareto.toConcreteMdp(manager);
        concreteMdps.push_back(std::static_pointer_cast<OpenMdp<ValueType>>(concreteMdp));
    }

    // 3)
    SumModel<ValueType> newSequenceModel(manager, concreteMdps);
    FlatMdpBuilderVisitor<ValueType> flatBuilder(manager);
    newSequenceModel.accept(flatBuilder);

    ConcreteMdp<ValueType> stitchedMdp = flatBuilder.getCurrent();
    visitConcreteModel(stitchedMdp);
}

template<typename ValueType>
void ParetoVisitor<ValueType>::visitTraceModel(TraceModel<ValueType>& model) {
    // 1) Get Pareto result for the child
    // 2) Turn Pareto result into a MDP
    // 3) Stitch them together, using code in the FlatMDP builder.

    // 1)
    model.value->accept(*this);

    // 2)
    std::shared_ptr<ConcreteMdp<ValueType>> concreteMdp = currentPareto.toConcreteMdp(manager);

    // 3)
    TraceModel<ValueType> newTraceModel(manager, concreteMdp, model.left, model.right);
    FlatMdpBuilderVisitor<ValueType> flatBuilder(manager);
    newTraceModel.accept(flatBuilder);

    ConcreteMdp<ValueType> stitchedMdp = flatBuilder.getCurrent();
    visitConcreteModel(stitchedMdp);
}

template<typename ValueType>
BidirectionalReachabilityResult<ValueType> ParetoVisitor<ValueType>::getCurrentPareto() {
    return currentPareto;
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
std::string ParetoVisitor<ValueType>::getFormula(ConcreteMdp<ValueType>& model, bool rewards) {
    std::stringstream formulaBuffer;
    formulaBuffer << "multi(\n";

    bool first = true;
    auto reachTarget = [&](const auto& l, bool leftExit) {
        std::string exitMarker = leftExit ? "lex" : "rex";

        size_t currentState = 0;
        for (const auto& v : l) {
            if (!first) {
                formulaBuffer << ", ";
            } else {
                formulaBuffer << "  ";
            }

            formulaBuffer << "Pmax=? [F \"" << exitMarker << currentState << "\"]";
            if (rewards) {
                formulaBuffer << ", Rmax=? [F \"" << exitMarker << currentState << "\"]";
            }
            first = false;
            formulaBuffer << "\n";
            ++currentState;
        }
    };
    reachTarget(model.lExit, true);
    reachTarget(model.rExit, false);

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
