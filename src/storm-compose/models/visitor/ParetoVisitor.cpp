#include "ParetoVisitor.h"

#include "storage/prism/Program.h"
#include "storm-parsers/api/storm-parsers.h"
#include "storm-parsers/parser/FormulaParser.h"
#include "storm/api/storm.h"
#include "storm/environment/modelchecker/MultiObjectiveModelCheckerEnvironment.h"
#include "storm/logic/Formula.h"
#include "storm/modelchecker/multiobjective/multiObjectiveModelChecking.h"
#include "storm/modelchecker/results/ExplicitParetoCurveCheckResult.h"
#include "storm/storage/jani/Property.h"

#include <memory>

namespace storm {
namespace models {
namespace visitor {

using storm::models::OpenMdp;
using storm::models::sparse::Mdp;
using storm::storage::SparseMatrix;
using storm::storage::SparseMatrixBuilder;

template<typename ValueType>
ParetoVisitor<ValueType>::ParetoVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager) : manager(manager) {
    auto& multiObjectiveOptions = env.modelchecker().multi();
    multiObjectiveOptions.setMethod(storm::modelchecker::multiobjective::MultiObjectiveMethod::Pcaa);
    // multiObjectiveOptions.setPrecision(0.1);
    // multiObjectiveOptions.setPrecisionType(storm::MultiObjectiveModelCheckerEnvironment::ABSOLUTE);
}

// TODO merge visitPrismModel and visitConcreteModel so that the code can be
// reused for example, by making it so the visitPrismModel simply constructs the
// concreteModel and then calls visitConcreteModel on it.
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

    std::cout << "Path: " << model.getPath() << std::endl;
    std::cout << "lEntrance: " << model.getLEntrance().size() << std::endl;
    std::cout << "rEntrance: " << model.getREntrance().size() << std::endl;
    std::cout << "lExit: " << model.getLExit().size() << std::endl;
    std::cout << "rExit: " << model.getRExit().size() << std::endl;

    BidirectionalReachabilityResult<ValueType> results(model.getLEntrance().size(), model.getREntrance().size(), model.getLExit().size(),
                                                       model.getRExit().size());
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
                STORM_LOG_ASSERT(model.getLExit().size() + model.getRExit().size() == 1, "Expected only 1 exit");
                auto quantitativeResult = result->template asExplicitQuantitativeCheckResult<ValueType>();

                std::cout << "Warning: Still need to fix this!!! (a)" << std::endl;
                // TODO below we assume that the initial state is 0, but this may not be the case (?)
                results.addPoint(entranceNumber, leftEntrance, {quantitativeResult[0]});
            }

            entranceNumber++;
        }
    };

    checkEntrances(model.getLEntrance(), true);
    checkEntrances(model.getREntrance(), false);

    currentPareto = results;
}

template<typename ValueType>
void ParetoVisitor<ValueType>::visitConcreteModel(ConcreteMdp<ValueType>& model) {
    std::string formulaString = getFormula(model);
    storm::parser::FormulaParser formulaParser;
    auto formula = formulaParser.parseSingleFormulaFromString(formulaString);

    BidirectionalReachabilityResult<ValueType> results(model.getLEntrance().size(), model.getREntrance().size(), model.getLExit().size(),
                                                       model.getRExit().size());
    size_t entranceNumber = 0;

    auto& stateLabeling = model.getMdp()->getStateLabeling();
    if (!stateLabeling.containsLabel("init")) {
        stateLabeling.addLabel("init");
    }

    auto checkEntrances = [&](const auto& entrances, bool leftEntrance) {
        for (auto const& entrance : entrances) {
            stateLabeling.addLabelToState("init", entrance);

            std::unique_ptr<storm::modelchecker::CheckResult> result =
                storm::modelchecker::multiobjective::performMultiObjectiveModelChecking(this->env, *model.getMdp(), formula->asMultiObjectiveFormula());
            stateLabeling.removeLabelFromState("init", entrance);

            if (result->isExplicitParetoCurveCheckResult()) {
                auto paretoResult = result->template asExplicitParetoCurveCheckResult<ValueType>();
                const auto& points = paretoResult.getPoints();

                for (size_t j = 0; j < points.size(); ++j) {
                    const auto& point = points[j];
                    results.addPoint(entranceNumber, leftEntrance, point);
                }
            } else {
                STORM_LOG_ASSERT(result->isExplicitQuantitativeCheckResult(), "result was not pareto nor quantitative");
                STORM_LOG_ASSERT(model.getLExit().size() + model.getRExit().size() == 1, "Expected only 1 exit");
                auto quantitativeResult = result->template asExplicitQuantitativeCheckResult<ValueType>();
                std::cout << "Warning: Still need to fix this!!! (b)" << std::endl;

                std::vector<ValueType> point{quantitativeResult[0]};
                results.addPoint(entranceNumber, leftEntrance, point);
            }

            entranceNumber++;
        }
    };

    checkEntrances(model.getLEntrance(), true);
    checkEntrances(model.getREntrance(), false);

    currentPareto = results;
}

template<typename ValueType>
void ParetoVisitor<ValueType>::visitReference(Reference<ValueType>& reference) {
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
        dereferenced->accept(*this);  // after this currentPareto should be set

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
    size_t count = 0;
    std::vector<std::shared_ptr<OpenMdp<ValueType>>> concreteMdps;
    for (const auto& openMdp : model.getValues()) {
        openMdp->accept(*this);

        // 2)
        std::shared_ptr<ConcreteMdp<ValueType>> concreteMdp = currentPareto.toConcreteMdp(manager);
        concreteMdps.push_back(std::static_pointer_cast<OpenMdp<ValueType>>(concreteMdp));

        // bool exportToDot = false; // TODO remove me
        // if (exportToDot) {
        //     std::string name = "model" + std::to_string(count) + ".dot";
        //     std::ofstream out(name);
        //     concreteMdp->getMdp()->writeDotToStream(out);
        // }
        ++count;
    }

    // 3)
    SumModel<ValueType> newSequenceModel(manager, concreteMdps);
    FlatMdpBuilderVisitor<ValueType> flatBuilder(manager);
    newSequenceModel.accept(flatBuilder);

    ConcreteMdp<ValueType> stitchedMdp = flatBuilder.getCurrent();

    // bool exportToDot = fals; // TODO remove me
    // if (exportToDot) {
    //     std::ofstream out("sum.dot");
    //     stitchedMdp.getMdp()->writeDotToStream(out);
    // }

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
std::string ParetoVisitor<ValueType>::getFormula(PrismModel<ValueType> const& model, bool rewards) {
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
    reachTarget(model.getLExit());
    reachTarget(model.getRExit());

    formulaBuffer << ")";

    return formulaBuffer.str();
}

template<typename ValueType>
std::string ParetoVisitor<ValueType>::getFormula(ConcreteMdp<ValueType> const& model, bool rewards) {
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
    reachTarget(model.getLExit(), true);
    reachTarget(model.getRExit(), false);

    formulaBuffer << ")";

    return formulaBuffer.str();
}

template<typename ValueType>
std::unordered_map<std::string, storm::expressions::Expression> ParetoVisitor<ValueType>::getIdentifierMapping(
    storm::expressions::ExpressionManager const& manager) {
    std::unordered_map<std::string, storm::expressions::Expression> result;
    for (const auto& var : manager.getVariables()) {
        result[var.getName()] = var.getExpression();
    }
    return result;
}

template class ParetoVisitor<double>;
template class ParetoVisitor<storm::RationalNumber>;

}  // namespace visitor
}  // namespace models
}  // namespace storm
