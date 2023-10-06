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
#include "storm-compose-cli/settings/modules/ComposeIOSettings.h"

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
    using PT = storm::MultiObjectiveModelCheckerEnvironment::PrecisionType;

    auto& multiObjectiveOptions = env.modelchecker().multi();
    multiObjectiveOptions.setMethod(storm::modelchecker::multiobjective::MultiObjectiveMethod::Pcaa);

    auto const& composeSettings = storm::settings::getModule<storm::settings::modules::ComposeIOSettings>();
    if (composeSettings.isParetoPrecisionSet()) {
        multiObjectiveOptions.setPrecision(composeSettings.getParetoPrecision());
    }
    auto newPrecisionType = PT::Absolute;
    if (composeSettings.isParetoPrecisionTypeSet()) {
        std::string precisionType = composeSettings.getParetoPrecisionType();
        if (precisionType == "absolute") {
            newPrecisionType = PT::Absolute;
        } else if (precisionType == "relative") {
            newPrecisionType = PT::RelativeToDiff;
        } else {
            STORM_LOG_THROW(false, storm::exceptions::NotSupportedException, "unknown precision type");
        }
    }
    multiObjectiveOptions.setPrecisionType(newPrecisionType);
    if (composeSettings.isParetoStepsSet()) multiObjectiveOptions.setMaxSteps(composeSettings.getParetoSteps());
}

// TODO merge visitPrismModel and visitConcreteModel so that the code can be
// reused for example, by making it so the visitPrismModel simply constructs the
// concreteModel and then calls visitConcreteModel on it.
template<typename ValueType>
void LowerUpperParetoVisitor<ValueType>::visitPrismModel(PrismModel<ValueType>& model) {
   STORM_LOG_ASSERT(false, "concretize MDPs first!");
}

template<typename ValueType>
void LowerUpperParetoVisitor<ValueType>::visitConcreteModel(ConcreteMdp<ValueType>& model) {
    //std::cout << "Visiting ConcreteModel" << std::endl;

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

                // TODO double check below
                std::vector<ValueType> point {quantitativeResult[entrance]};
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
    //std::cout << "Visiting Reference" << std::endl;

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
    //std::cout << "Visiting SequenceModel" << std::endl;

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
    //std::cout << "Visiting SumModel" << std::endl;

    // The sum of two pareto curves is simply extending each point with zeroes
    // for the entries that do not belong to the child

    typename storm::models::OpenMdp<ValueType>::Scope emptyScope;
    size_t lEntrances = model.collectEntranceExit(storm::models::OpenMdp<ValueType>::L_ENTRANCE, emptyScope).size();
    size_t rEntrances = model.collectEntranceExit(storm::models::OpenMdp<ValueType>::R_ENTRANCE, emptyScope).size();
    size_t lExits = model.collectEntranceExit(storm::models::OpenMdp<ValueType>::L_EXIT, emptyScope).size();
    size_t rExits = model.collectEntranceExit(storm::models::OpenMdp<ValueType>::R_EXIT, emptyScope).size();

    std::vector<decltype(currentPareto)> paretoCurves;
    for (const auto& openMdp : model.getValues()) {
        openMdp->accept(*this);
        paretoCurves.push_back(currentPareto);
    }

    size_t pointDimension = lExits+rExits;
    BidirectionalReachabilityResult<ValueType> 
        lower(lEntrances, rEntrances, lExits, rExits),
        upper(lEntrances, rEntrances, lExits, rExits);

    size_t entranceOffset = 0, exitOffset = 0;
    for (const auto &curve : paretoCurves) {
        auto processPoints = [&](const auto& result, bool leftEntrance, auto& target) {
            size_t entranceCount = leftEntrance ? result.getLeftEntrances() : result.getRightEntrances();
            for (size_t entrance = 0; entrance < entranceCount; ++entrance) {
                const auto& points = result.getPoints(entrance, leftEntrance);
                for (const auto &point : points) {
                    std::vector<ValueType> newPoint;
                    for (size_t i = 0; i < exitOffset; ++i) {
                        newPoint.push_back(storm::utility::zero<ValueType>());
                    }

                    for (const auto& v : point) {
                        newPoint.push_back(v);
                    }

                    for (size_t i = newPoint.size(); i < pointDimension; ++i) {
                        newPoint.push_back(storm::utility::zero<ValueType>());
                    }

                    target.addPoint(entranceOffset + entrance, leftEntrance, newPoint);
                }
            }
        };

        // TODO deal with right entrances too
        processPoints(curve.first, true, lower);
        //processPoints(curve.first, false, lower);
        processPoints(curve.second, true, upper);
        //processPoints(curve.second, false, upper);

        STORM_LOG_ASSERT(curve.first.getLeftEntrances() == curve.second.getLeftEntrances(), "sanity check");
        STORM_LOG_ASSERT(curve.first.getPointDimension() == curve.second.getPointDimension(), "sanity check");

        entranceOffset += curve.first.getLeftEntrances();
        exitOffset += curve.first.getPointDimension();
    }

    currentPareto = {lower, upper};
}

template<typename ValueType>
void LowerUpperParetoVisitor<ValueType>::visitTraceModel(TraceModel<ValueType>& model) {
    //std::cout << "Visiting TraceModel" << std::endl;
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
