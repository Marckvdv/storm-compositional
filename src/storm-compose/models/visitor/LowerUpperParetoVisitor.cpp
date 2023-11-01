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
#include "storm-compose/models/visitor/FlatMdpBuilderVisitor.h"

#include <memory>

namespace storm {
namespace models {
namespace visitor {

using storm::models::OpenMdp;
using storm::storage::SparseMatrixBuilder;
using storm::storage::SparseMatrix;
using storm::models::sparse::Mdp;

template<typename ValueType>
std::shared_ptr<storm::storage::geometry::Polytope<ValueType>> getPositivityPolytope(size_t dimension) {
    std::vector<storm::storage::geometry::Halfspace<ValueType>> halfSpaces;
    for (size_t i = 0; i < dimension; ++i) {
        std::vector<ValueType> normal(dimension, 0);
        normal[i] = -1;
        halfSpaces.push_back(storm::storage::geometry::Halfspace<ValueType>(normal, 0));
    }

    return storm::storage::geometry::Polytope<ValueType>::create(halfSpaces);
}

template<typename ValueType>
std::shared_ptr<storm::storage::geometry::Polytope<ValueType>> getProbabilitySimplex(size_t dimension) {
    std::vector<storm::storage::geometry::Halfspace<ValueType>> halfSpaces;
    // each value must be greater or equal to zero
    for (size_t i = 0; i < dimension; ++i) {
        std::vector<ValueType> normal(dimension, 0);
        normal[i] = -1;
        halfSpaces.push_back(storm::storage::geometry::Halfspace<ValueType>(normal, 0));
    }
    // the sum cannot be more than 1
    std::vector<ValueType> normal(dimension, 1);
    halfSpaces.push_back(storm::storage::geometry::Halfspace<ValueType>(normal, 1));

    return storm::storage::geometry::Polytope<ValueType>::create(halfSpaces);
}

template<typename ValueType>
LowerUpperParetoVisitor<ValueType>::LowerUpperParetoVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager, storm::compose::benchmark::BenchmarkStats<ValueType>& stats, LowerUpperParetoSettings settings) : manager(manager), stats(stats), settings(settings) {
    using PT = storm::MultiObjectiveModelCheckerEnvironment::PrecisionType;

    auto& multiObjectiveOptions = env.modelchecker().multi();
    multiObjectiveOptions.setMethod(storm::modelchecker::multiobjective::MultiObjectiveMethod::Pcaa);

    multiObjectiveOptions.setPrecision(settings.precision);
    auto newPrecisionType = PT::Absolute;
    if (settings.precisionType == "absolute") {
        newPrecisionType = PT::Absolute;
    } else if (settings.precisionType == "relative") {
        newPrecisionType = PT::RelativeToDiff;
    } else {
        STORM_LOG_THROW(false, storm::exceptions::NotSupportedException, "unknown precision type");
    }
    multiObjectiveOptions.setPrecisionType(newPrecisionType);
    if (settings.steps) multiObjectiveOptions.setMaxSteps(*settings.steps);
}

// TODO merge visitPrismModel and visitConcreteModel so that the code can be
// reused for example, by making it so the visitPrismModel simply constructs the
// concreteModel and then calls visitConcreteModel on it.
template<typename ValueType>
void LowerUpperParetoVisitor<ValueType>::visitPrismModel(PrismModel<ValueType>& model) {
   STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "concretize MDPs first!");
}

template<typename ValueType>
void LowerUpperParetoVisitor<ValueType>::visitConcreteModel(ConcreteMdp<ValueType>& model) {
    STORM_LOG_DEBUG("[COMPOSE] Visiting ConcreteModel");

    //std::ofstream f("m.dot");
    //model.getMdp()->writeDotToStream(f);
    //f.close();

    std::string formulaString = getFormula(model);
    storm::parser::FormulaParser formulaParser;
    auto formula = formulaParser.parseSingleFormulaFromString(formulaString);

    BidirectionalReachabilityResult<ValueType> lowerResults(model.lEntrance.size(), model.rEntrance.size(), model.lExit.size(), model.rExit.size()),
        upperResults(model.lEntrance.size(), model.rEntrance.size(), model.lExit.size(), model.rExit.size());

    auto& stateLabeling = model.getMdp()->getStateLabeling();
    if (!stateLabeling.containsLabel("init")) {
        stateLabeling.addLabel("init");
    } else {
        stateLabeling.setStates("init", storm::storage::BitVector(model.getMdp()->getNumberOfStates()));
    }

    auto checkEntrances = [&](const auto& entrances, bool leftEntrance) {
        size_t entranceNumber = 0;
        for (auto const& entrance : entrances) {
            stateLabeling.addLabelToState("init", entrance);

            stats.reachabilityComputationTime.start();
            std::unique_ptr<storm::modelchecker::CheckResult> result =
                storm::modelchecker::multiobjective::performMultiObjectiveModelChecking(this->env, *model.getMdp(), formula->asMultiObjectiveFormula());
            stats.reachabilityComputationTime.stop();
            stateLabeling.removeLabelFromState("init", entrance);

            if (result->isExplicitParetoCurveCheckResult()) {
                auto paretoResult = result->template asExplicitParetoCurveCheckResult<ValueType>();

                STORM_LOG_THROW(paretoResult.hasUnderApproximation(), storm::exceptions::InvalidOperationException, "expected under approximation");
                STORM_LOG_THROW(paretoResult.hasOverApproximation(), storm::exceptions::InvalidOperationException, "expected over approximation");

                //std::cout << "Upper verts before intersection: " << std::endl;
                //for (auto &p : paretoResult.getOverApproximation()->getVertices()) {
                //    for (auto& v : p) {
                //        std::cout << v << ", ";
                //    }
                //    std::cout << std::endl;
                //}

                auto probabilisticSimplex = getProbabilitySimplex<ValueType>(lowerResults.getPointDimension());
                auto lowerPolytope = paretoResult.getUnderApproximation()->intersection(probabilisticSimplex);
                const auto& lowerPoints = lowerPolytope->getVertices();

                auto upperPolytope = paretoResult.getOverApproximation()->intersection(probabilisticSimplex);
                const auto& upperPoints = upperPolytope->getVertices();

                this->stats.paretoPoints += lowerPoints.size();
                this->stats.paretoPoints += upperPoints.size();

                //std::cout << "Lower verts after intersection: " << std::endl;
                //for (auto &p : lowerPoints) {
                //    for (auto& v : p) {
                //        std::cout << v << ", ";
                //    }
                //    std::cout << std::endl;
                //}
                //const auto& lowerPoints = paretoResult.getUnderApproximation()->getVertices();
                //const auto& upperPoints = paretoResult.getOverApproximation()->getVertices();

                for (size_t j = 0; j < lowerPoints.size(); ++j) {
                    const auto& point = lowerPoints[j];
                    lowerResults.addPoint(entranceNumber, leftEntrance, point);
                }

                for (size_t j = 0; j < upperPoints.size(); ++j) {
                    const auto& point = upperPoints[j];
                    upperResults.addPoint(entranceNumber, leftEntrance, point);
                }
            } else {
                STORM_LOG_THROW(result->isExplicitQuantitativeCheckResult(), storm::exceptions::InvalidOperationException, "result was not pareto nor quantitative");
                STORM_LOG_THROW(model.lExit.size() + model.rExit.size() == 1, storm::exceptions::InvalidOperationException, "Expected only 1 exit");
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

    STORM_LOG_DEBUG("[COMPOSE] Existing ConcreteModel");
}

template<typename ValueType>
void LowerUpperParetoVisitor<ValueType>::visitReference(Reference<ValueType>& reference) {
    // Perform caching if possible
    const auto& referenceName = reference.getReference();
    STORM_LOG_DEBUG("[COMPOSE] Visiting Reference " << referenceName);

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

    STORM_LOG_DEBUG("[COMPOSE] Exiting Reference " << referenceName);
}

template<typename ValueType>
void LowerUpperParetoVisitor<ValueType>::visitSequenceModel(SequenceModel<ValueType>& model) {
    STORM_LOG_DEBUG("[COMPOSE] Visiting SequenceModel");

    // 1) Get Pareto result for each child
    // 2) Turn each Pareto result into a MDP
    // 3) Stitch them together, using code in the FlatMDP builder.

    // 1)
    std::vector<std::shared_ptr<OpenMdp<ValueType>>> concreteMdpsLower, concreteMdpsUpper;
    for (auto& openMdp : model.getValues()) {
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

    STORM_LOG_DEBUG("[COMPOSE] Exiting SequenceModel");
}

template<typename ValueType>
void LowerUpperParetoVisitor<ValueType>::visitSumModel(SumModel<ValueType>& model) {
    STORM_LOG_DEBUG("[COMPOSE] Visiting SumModel");

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

    size_t entranceOffsetLeft = 0, entranceOffsetRight = 0, exitOffset = 0;
    for (const auto &curve : paretoCurves) {
        auto processPoints = [&](const auto& result, bool leftEntrance, auto& target) {
            size_t entranceCount = leftEntrance ? result.getLeftEntrances() : result.getRightEntrances();
            size_t entranceOffset = leftEntrance ? entranceOffsetLeft : entranceOffsetRight;
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

        processPoints(curve.first, true, lower);
        processPoints(curve.first, false, lower);
        processPoints(curve.second, true, upper);
        processPoints(curve.second, false, upper);

        STORM_LOG_ASSERT(curve.first.getLeftEntrances() == curve.second.getLeftEntrances(), "sanity check");
        STORM_LOG_ASSERT(curve.first.getRightEntrances() == curve.second.getRightEntrances(), "sanity check");
        STORM_LOG_ASSERT(curve.first.getPointDimension() == curve.second.getPointDimension(), "sanity check");

        entranceOffsetLeft += curve.first.getLeftEntrances();
        entranceOffsetRight += curve.first.getRightEntrances();
        exitOffset += curve.first.getPointDimension();
    }

    currentPareto = {lower, upper};

    STORM_LOG_DEBUG("[COMPOSE] Exiting SumModel");
}

template<typename ValueType>
void LowerUpperParetoVisitor<ValueType>::visitTraceModel(TraceModel<ValueType>& model) {
    STORM_LOG_DEBUG("[COMPOSE] Visiting TraceModel");
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
    STORM_LOG_DEBUG("[COMPOSE] Exiting TraceModel");
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
