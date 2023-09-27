#include "LowerUpperParetoVisitor.h"

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
LowerUpperParetoVisitor<ValueType>::LowerUpperParetoVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager) : manager(manager) {
    auto& multiObjectiveOptions = env.modelchecker().multi();
    multiObjectiveOptions.setMethod(storm::modelchecker::multiobjective::MultiObjectiveMethod::Pcaa);
    //multiObjectiveOptions.setPrecision(0.1);
    //multiObjectiveOptions.setPrecisionType(storm::MultiObjectiveModelCheckerEnvironment::ABSOLUTE);
}

// TODO merge visitPrismModel and visitConcreteModel so that the code can be
// reused for example, by making it so the visitPrismModel simply constructs the
// concreteModel and then calls visitConcreteModel on it.
template<typename ValueType>
void LowerUpperParetoVisitor<ValueType>::visitPrismModel(PrismModel<ValueType>& model) {
    /*
    auto program = storm::api::parseProgram(model.getPath(), false, false);

    std::string formulaString = getFormula(model);
    std::vector<std::shared_ptr<storm::logic::Formula const>> formulas =
        storm::api::extractFormulasFromProperties(storm::api::parsePropertiesForPrismProgram(formulaString, program));
    STORM_LOG_ASSERT(formulas.size() == 1, "sanity check");
    auto formula = formulas.at(0);

    const auto& manager = program.getManager();
    storm::parser::ExpressionParser parser(manager);
    parser.setIdentifierMapping(getIdentifierMapping(manager));

    BidirectionalReachabilityResult<ValueType> results(model.lEntrance.size(), model.rEntrance.size(), model.lExit.size(), model.rExit.size());
    auto checkEntrances = [&](const auto& entrances, bool leftEntrance) {
        // TODO find away we do not have to rebuild the MDP for each entrance.
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
                STORM_LOG_ASSERT(result->isExplicitQuantitativeCheckResult(), "result was not pareto nor quantitative");
                STORM_LOG_ASSERT(model.lExit.size() + model.rExit.size() == 1, "Expected only 1 exit");
                auto quantitativeResult = result->template asExplicitQuantitativeCheckResult<ValueType>();

                std::cout << "Warning: Still need to fix this!!! (a)" << std::endl;
                // TODO below we assume that the initial state is 0, but this may not be the case (?)
                results.addPoint(entranceNumber, leftEntrance, {quantitativeResult[0]});
            }

            entranceNumber++;
        }
    };

    checkEntrances(model.lEntrance, true);
    checkEntrances(model.rEntrance, false);

    currentPareto = results;
    */
   STORM_LOG_ASSERT(false, "concretize MDPs first!");
}

template<typename ValueType>
void LowerUpperParetoVisitor<ValueType>::visitConcreteModel(ConcreteMdp<ValueType>& model) {
    std::string formulaString = getFormula(model);
    storm::parser::FormulaParser formulaParser;
    auto formula = formulaParser.parseSingleFormulaFromString(formulaString);

    BidirectionalReachabilityResult<ValueType> lowerResults(model.lEntrance.size(), model.rEntrance.size(), model.lExit.size(), model.rExit.size()),
        upperResults(model.lEntrance.size(), model.rEntrance.size(), model.lExit.size(), model.rExit.size());
    size_t entranceNumber = 0;

    auto& stateLabeling = model.getMdp()->getStateLabeling();
    if (!stateLabeling.containsLabel("init")) {
        stateLabeling.addLabel("init");
    } else {
        stateLabeling.setStates("init", storm::storage::BitVector(model.getMdp()->getNumberOfStates()));
    }

    auto checkEntrances = [&](const auto& entrances, bool leftEntrance) {
        for (auto const& entrance : entrances) {
            stateLabeling.addLabelToState("init", entrance);

            std::unique_ptr<storm::modelchecker::CheckResult> result =
                storm::modelchecker::multiobjective::performMultiObjectiveModelChecking(this->env, *model.getMdp(), formula->asMultiObjectiveFormula());
            stateLabeling.removeLabelFromState("init", entrance);

            if (result->isExplicitParetoCurveCheckResult()) {
                auto paretoResult = result->template asExplicitParetoCurveCheckResult<ValueType>();

                STORM_LOG_ASSERT(paretoResult.hasUnderApproximation(), "expected under approximation");
                STORM_LOG_ASSERT(paretoResult.hasOverApproximation(), "expected over approximation");

                const auto& lowerPoints = paretoResult.getUnderApproximation()->getVertices();
                const auto& upperPoints = paretoResult.getOverApproximation()->getVertices();

                for (size_t j = 0; j < lowerPoints.size(); ++j) {
                    const auto& point = lowerPoints[j];
                    lowerResults.addPoint(entranceNumber, leftEntrance, point);
                }

                for (size_t j = 0; j < upperPoints.size(); ++j) {
                    const auto& point = upperPoints[j];
                    upperResults.addPoint(entranceNumber, leftEntrance, point);
                }
            } else {
                STORM_LOG_ASSERT(result->isExplicitQuantitativeCheckResult(), "result was not pareto nor quantitative");
                STORM_LOG_ASSERT(model.lExit.size() + model.rExit.size() == 1, "Expected only 1 exit");
                auto quantitativeResult = result->template asExplicitQuantitativeCheckResult<ValueType>();
                std::cout << "Warning: Still need to fix this!!! (b)" << std::endl;

                std::vector<ValueType> point {quantitativeResult[0]};
                lowerResults.addPoint(entranceNumber, leftEntrance, point);
                upperResults.addPoint(entranceNumber, leftEntrance, point);
            }

            entranceNumber++;
        }
    };

    checkEntrances(model.lEntrance, true);
    checkEntrances(model.rEntrance, false);

    currentPareto = {lowerResults, upperResults};
}

template<typename ValueType>
void LowerUpperParetoVisitor<ValueType>::visitReference(Reference<ValueType>& reference) {
    // Perform caching if possible
    const auto& referenceName = reference.getReference();
    auto it = paretoResults.find(referenceName);
    if (it != paretoResults.end()) {
        // TODO avoid copying
        currentPareto = it->second;
    } else {
        // Result is not yet cached so we need to compute it ourself
        const auto manager = reference.getManager();
        auto dereferenced = manager->dereference(referenceName);
        dereferenced->accept(*this); // after this currentPareto should be set

        paretoResults[referenceName] = currentPareto;
    }
}

template<typename ValueType>
void LowerUpperParetoVisitor<ValueType>::visitSequenceModel(SequenceModel<ValueType>& model) {
    // 1) Get Pareto result for each child
    // 2) Turn each Pareto result into a MDP
    // 3) Stitch them together, using code in the FlatMDP builder.

    // 1)
    std::vector<std::shared_ptr<OpenMdp<ValueType>>> concreteMdpsLower, concreteMdpsUpper;
    for (const auto& openMdp : model.getValues()) {
        openMdp->accept(*this);

        // 2)
        std::shared_ptr<ConcreteMdp<ValueType>> concreteMdpLower = currentPareto.first.toConcreteMdp(manager);
        concreteMdpsLower.push_back(std::static_pointer_cast<OpenMdp<ValueType>>(concreteMdpLower));

        std::shared_ptr<ConcreteMdp<ValueType>> concreteMdpUpper = currentPareto.second.toConcreteMdp(manager);
        concreteMdpsUpper.push_back(std::static_pointer_cast<OpenMdp<ValueType>>(concreteMdpUpper));
    }

    // 3)
    SequenceModel<ValueType> newSequenceModelLower(manager, concreteMdpsLower);
    FlatMdpBuilderVisitor<ValueType> flatBuilderLower(manager);
    newSequenceModelLower.accept(flatBuilderLower);

    ConcreteMdp<ValueType> stitchedMdpLower = flatBuilderLower.getCurrent();

    SequenceModel<ValueType> newSequenceModelUpper(manager, concreteMdpsUpper);
    FlatMdpBuilderVisitor<ValueType> flatBuilderUpper(manager);
    newSequenceModelUpper.accept(flatBuilderUpper);

    ConcreteMdp<ValueType> stitchedMdpUpper = flatBuilderUpper.getCurrent();

    visitConcreteModel(stitchedMdpLower);
    const auto resultLower = currentPareto;

    visitConcreteModel(stitchedMdpUpper);
    const auto resultUpper = currentPareto;

    currentPareto = {resultLower.first, resultUpper.second};
}

template<typename ValueType>
void LowerUpperParetoVisitor<ValueType>::visitSumModel(SumModel<ValueType>& model) {
    // 1) Get Pareto result for each child
    // 2) Turn each Pareto result into a MDP
    // 3) Stitch them together, using code in the FlatMDP builder.

    // 1)
    size_t count = 0;
    std::vector<std::shared_ptr<OpenMdp<ValueType>>> concreteMdpsLower, concreteMdpsUpper;
    for (const auto& openMdp : model.getValues()) {
        openMdp->accept(*this);

        // 2)
        auto concreteMdpLower = currentPareto.first.toConcreteMdp(manager);
        auto concreteMdpUpper = currentPareto.second.toConcreteMdp(manager);

        concreteMdpsLower.push_back(std::static_pointer_cast<OpenMdp<ValueType>>(concreteMdpLower));
        concreteMdpsUpper.push_back(std::static_pointer_cast<OpenMdp<ValueType>>(concreteMdpUpper));

        //bool exportToDot = true; // TODO remove me
        //if (exportToDot) {
        //    std::string name = "model" + std::to_string(count) + ".dot";
        //    std::ofstream out(name);
        //    concreteMdp.first->getMdp()->writeDotToStream(out);
        //}
        ++count;
    }

    // 3)
    SumModel<ValueType> lowerSum(manager, concreteMdpsLower), upperSum(manager, concreteMdpsUpper);

    FlatMdpBuilderVisitor<ValueType> flatBuilderLower(manager);
    lowerSum.accept(flatBuilderLower);
    ConcreteMdp<ValueType> stitchedLower = flatBuilderLower.getCurrent();

    FlatMdpBuilderVisitor<ValueType> flatBuilderUpper(manager);
    upperSum.accept(flatBuilderUpper);
    ConcreteMdp<ValueType> stitchedUpper = flatBuilderUpper.getCurrent();

    //bool exportToDot = true; // TODO remove me
    //if (exportToDot) {
    //    std::ofstream out("sum.dot");
    //    stitchedMdp.getMdp()->writeDotToStream(out);
    //}

    // TODO combine two results below
    visitConcreteModel(stitchedLower);
    auto lowerResult = currentPareto;

    visitConcreteModel(stitchedUpper);
    auto upperResult = currentPareto;

    // Update new lower and upper bound to (lower.lower, upper.upper)
    currentPareto = {lowerResult.first, upperResult.second};
}

template<typename ValueType>
void LowerUpperParetoVisitor<ValueType>::visitTraceModel(TraceModel<ValueType>& model) {
    // 1) Get Pareto result for the child
    // 2) Turn Pareto result into a MDP
    // 3) Stitch them together, using code in the FlatMDP builder.

    // 1)
    model.value->accept(*this);

    // 2)
    std::shared_ptr<ConcreteMdp<ValueType>> concreteMdpLower = currentPareto.first.toConcreteMdp(manager);
    std::shared_ptr<ConcreteMdp<ValueType>> concreteMdpUpper = currentPareto.second.toConcreteMdp(manager);

    // 3)
    TraceModel<ValueType> newTraceModelLower(manager, concreteMdpLower, model.left, model.right);
    FlatMdpBuilderVisitor<ValueType> flatBuilderLower(manager);
    newTraceModelLower.accept(flatBuilderLower);

    ConcreteMdp<ValueType> stitchedMdpLower = flatBuilderLower.getCurrent();

    TraceModel<ValueType> newTraceModelUpper(manager, concreteMdpUpper, model.left, model.right);
    FlatMdpBuilderVisitor<ValueType> flatBuilderUpper(manager);
    newTraceModelUpper.accept(flatBuilderUpper);

    ConcreteMdp<ValueType> stitchedMdpUpper = flatBuilderUpper.getCurrent();

    visitConcreteModel(stitchedMdpLower);
    const auto lowerResult = currentPareto;

    visitConcreteModel(stitchedMdpUpper);
    const auto upperResult = currentPareto;

    currentPareto = {lowerResult.first, upperResult.second};
}

template<typename ValueType>
std::pair<BidirectionalReachabilityResult<ValueType>, BidirectionalReachabilityResult<ValueType>> LowerUpperParetoVisitor<ValueType>::getCurrentPareto() {
    return currentPareto;
}

template<typename ValueType>
std::string LowerUpperParetoVisitor<ValueType>::getFormula(PrismModel<ValueType> const& model, bool rewards) {
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
std::string LowerUpperParetoVisitor<ValueType>::getFormula(ConcreteMdp<ValueType> const& model, bool rewards) {
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
std::unordered_map<std::string, storm::expressions::Expression> LowerUpperParetoVisitor<ValueType>::getIdentifierMapping(storm::expressions::ExpressionManager const& manager) {
    std::unordered_map<std::string, storm::expressions::Expression> result;
    for (const auto& var : manager.getVariables()) {
        result[var.getName()] = var.getExpression();
    }
    return result;
}

template class LowerUpperParetoVisitor<double>;
template class LowerUpperParetoVisitor<storm::RationalNumber>;

}
}
}
