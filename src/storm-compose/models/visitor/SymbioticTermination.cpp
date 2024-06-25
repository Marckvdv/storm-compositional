#include "SymbioticTermination.h"

#include "exceptions/NotSupportedException.h"
#include "storage/prism/Program.h"
#include "storm-compose-cli/settings/modules/ComposeIOSettings.h"
#include "storm-compose/models/visitor/EntranceExitVisitor.h"
#include "storm-compose/models/visitor/FlatMdpBuilderVisitor.h"
#include "storm-compose/models/visitor/LowerUpperParetoVisitor.h"
#include "storm-parsers/api/storm-parsers.h"
#include "storm-parsers/parser/FormulaParser.h"
#include "storm/api/storm.h"
#include "storm/environment/modelchecker/MultiObjectiveModelCheckerEnvironment.h"
#include "storm/logic/Formula.h"
#include "storm/modelchecker/multiobjective/multiObjectiveModelChecking.h"
#include "storm/modelchecker/multiobjective/pcaa/SparsePcaaParetoQuery.h"
#include "storm/modelchecker/multiobjective/preprocessing/SparseMultiObjectivePreprocessor.h"
#include "storm/modelchecker/results/ExplicitParetoCurveCheckResult.h"
#include "storm/storage/jani/Property.h"

#include <memory>

namespace storm {
namespace models {
namespace visitor {

using storm::models::OpenMdp;
using storm::models::sparse::Mdp;

template<typename ValueType>
bool isDominating(std::vector<ValueType>& a, std::vector<ValueType>& b) {
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] < b[i])
            return false;
    }
    return true;
}

template<typename ValueType>
void removeDominatedPoints(std::vector<std::vector<ValueType>>& points) {
    for (int64_t i = 0; i < points.size(); ++i) {
        for (int64_t j = 0; j < points.size(); ++j) {
            if (i == j)
                continue;

            if (isDominating(points[j], points[i])) {
                points.erase(points.begin() + i);
                --i;
                goto nextpoint;
            }
        }
    nextpoint:;
    }
}

template<typename ValueType>
void removeDominatingPoints(std::vector<std::vector<ValueType>>& points) {
    for (int64_t i = 0; i < points.size(); ++i) {
        for (int64_t j = 0; j < points.size(); ++j) {
            if (i == j)
                continue;

            if (isDominating(points[i], points[j])) {
                points.erase(points.begin() + i);
                --i;
                goto nextpoint;
            }
        }
    nextpoint:;
    }
}

template<typename ValueType>
SymbioticTermination<ValueType>::SymbioticTermination(std::shared_ptr<OpenMdpManager<ValueType>> manager,
                                                      storm::compose::benchmark::BenchmarkStats<ValueType>& stats, LowerUpperParetoSettings settings,
                                                      storage::ParetoCache<ValueType>& paretoCache)
    : manager(manager), stats(stats), settings(settings), paretoCache(paretoCache) {
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
    if (settings.steps)
        multiObjectiveOptions.setMaxSteps(*settings.steps);
}

// TODO merge visitPrismModel and visitConcreteModel so that the code can be
// reused for example, by making it so the visitPrismModel simply constructs the
// concreteModel and then calls visitConcreteModel on it.
template<typename ValueType>
void SymbioticTermination<ValueType>::visitPrismModel(PrismModel<ValueType>& model) {
    STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "concretize MDPs first!");
}

template<typename ValueType>
void SymbioticTermination<ValueType>::visitConcreteModel(ConcreteMdp<ValueType>& model) {
    STORM_LOG_DEBUG("[COMPOSE] Visiting ConcreteModel");

    // std::ofstream f("m.dot");
    // model.getMdp()->writeDotToStream(f);
    // f.close();
    if (paretoCache.containsLeaf(&model)) {
        auto lower = paretoCache.getLowerBoundReachabilityResult(&model);
        auto upper = paretoCache.getUpperBoundReachabilityResult(&model);
        currentPareto = {lower, upper};

        return;
    }

    std::string formulaString = LowerUpperParetoVisitor<ValueType>::getFormula(model);
    storm::parser::FormulaParser formulaParser;
    auto formula = formulaParser.parseSingleFormulaFromString(formulaString);

    BidirectionalReachabilityResult<ValueType> lowerResults(model), upperResults(model);

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

                auto subdistributionPolytope = storm::storage::geometry::Polytope<ValueType>::getSubdistributionPolytope(lowerResults.getPointDimension());
                auto lowerPolytope = paretoResult.getUnderApproximation()->intersection(subdistributionPolytope);
                auto lowerPoints = lowerPolytope->getVertices();

                auto upperPolytope = paretoResult.getOverApproximation()->intersection(subdistributionPolytope);
                auto upperPoints = upperPolytope->getVertices();

                removeDominatedPoints<ValueType>(lowerPoints);
                // TODO fix below
                // removeDominatingPoints<ValueType>(upperPoints);

                this->stats.lowerParetoPoints += lowerPoints.size();
                this->stats.upperParetoPoints += upperPoints.size();

                for (size_t j = 0; j < lowerPoints.size(); ++j) {
                    const auto& point = lowerPoints[j];
                    lowerResults.addPoint(entranceNumber, leftEntrance, point);
                }

                for (size_t j = 0; j < upperPoints.size(); ++j) {
                    const auto& point = upperPoints[j];
                    upperResults.addPoint(entranceNumber, leftEntrance, point);
                }
            } else {
                STORM_LOG_THROW(result->isExplicitQuantitativeCheckResult(), storm::exceptions::InvalidOperationException,
                                "result was not pareto nor quantitative");
                STORM_LOG_THROW(model.getLExit().size() + model.getRExit().size() == 1, storm::exceptions::InvalidOperationException, "Expected only 1 exit");
                auto quantitativeResult = result->template asExplicitQuantitativeCheckResult<ValueType>();

                // TODO double check below
                std::vector<ValueType> point{quantitativeResult[entrance]};
                lowerResults.addPoint(entranceNumber, leftEntrance, point);
                upperResults.addPoint(entranceNumber, leftEntrance, point);
            }

            entranceNumber++;
        }
    };

    checkEntrances(model.getLEntrance(), true);
    checkEntrances(model.getREntrance(), false);

    currentPareto = {lowerResults, upperResults};

    STORM_LOG_DEBUG("[COMPOSE] Existing ConcreteModel");
}

template<typename ValueType>
void SymbioticTermination<ValueType>::visitReference(Reference<ValueType>& reference) {
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
        dereferenced->accept(*this);  // after this currentPareto should be set

        paretoResults[referenceName] = currentPareto;
    }

    STORM_LOG_DEBUG("[COMPOSE] Exiting Reference " << referenceName);
}

template<typename ValueType>
void SymbioticTermination<ValueType>::visitSequenceModel(SequenceModel<ValueType>& model) {
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
void SymbioticTermination<ValueType>::visitSumModel(SumModel<ValueType>& model) {
    STORM_LOG_DEBUG("[COMPOSE] Visiting SumModel");

    // The sum of two pareto curves is simply extending each point with zeroes
    // for the entries that do not belong to the child

    models::visitor::EntranceExitVisitor<ValueType> entranceExitVisitor;
    entranceExitVisitor.setEntranceExit(storage::L_ENTRANCE);
    model.accept(entranceExitVisitor);
    size_t lEntrances = entranceExitVisitor.getCollected().size();

    entranceExitVisitor.setEntranceExit(storage::R_ENTRANCE);
    model.accept(entranceExitVisitor);
    size_t rEntrances = entranceExitVisitor.getCollected().size();

    entranceExitVisitor.setEntranceExit(storage::L_EXIT);
    model.accept(entranceExitVisitor);
    size_t lExits = entranceExitVisitor.getCollected().size();

    entranceExitVisitor.setEntranceExit(storage::R_EXIT);
    model.accept(entranceExitVisitor);
    size_t rExits = entranceExitVisitor.getCollected().size();

    std::vector<decltype(currentPareto)> paretoCurves;
    for (const auto& openMdp : model.getValues()) {
        openMdp->accept(*this);
        paretoCurves.push_back(currentPareto);
    }

    size_t pointDimension = lExits + rExits;
    BidirectionalReachabilityResult<ValueType> lower(lEntrances, rEntrances, lExits, rExits), upper(lEntrances, rEntrances, lExits, rExits);

    size_t entranceOffsetLeft = 0, entranceOffsetRight = 0, exitOffset = 0;
    for (const auto& curve : paretoCurves) {
        auto processPoints = [&](const auto& result, bool leftEntrance, auto& target) {
            size_t entranceCount = leftEntrance ? result.getLeftEntrances() : result.getRightEntrances();
            size_t entranceOffset = leftEntrance ? entranceOffsetLeft : entranceOffsetRight;
            for (size_t entrance = 0; entrance < entranceCount; ++entrance) {
                const auto& points = result.getPoints(entrance, leftEntrance);
                for (const auto& point : points) {
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
void SymbioticTermination<ValueType>::visitTraceModel(TraceModel<ValueType>& model) {
    STORM_LOG_THROW(false, storm::exceptions::NotSupportedException, "Trace currently not supported");
    //    STORM_LOG_DEBUG("[COMPOSE] Visiting TraceModel");
    //    // 1) Get Pareto result for the child
    //    // 2) Turn Pareto result into a MDP
    //    // 3) Stitch them together, using code in the FlatMDP builder.
    //
    //    // 1)
    //    model.value->accept(*this);
    //
    //    // 2)
    //    std::shared_ptr<ConcreteMdp<ValueType>> concreteMdpLower = currentPareto.first.toConcreteMdp(manager);
    //    std::shared_ptr<ConcreteMdp<ValueType>> concreteMdpUpper = currentPareto.second.toConcreteMdp(manager);
    //
    //    // 3)
    //    TraceModel<ValueType> newTraceModelLower(manager, concreteMdpLower, model.left, model.right);
    //    FlatMdpBuilderVisitor<ValueType> flatBuilderLower(manager);
    //    newTraceModelLower.accept(flatBuilderLower);
    //    ConcreteMdp<ValueType> stitchedMdpLower = flatBuilderLower.getCurrent();
    //
    //    TraceModel<ValueType> newTraceModelUpper(manager, concreteMdpUpper, model.left, model.right);
    //    FlatMdpBuilderVisitor<ValueType> flatBuilderUpper(manager);
    //    newTraceModelUpper.accept(flatBuilderUpper);
    //    ConcreteMdp<ValueType> stitchedMdpUpper = flatBuilderUpper.getCurrent();
    //
    //    visitConcreteModel(stitchedMdpLower);
    //    const auto lowerResult = currentPareto;
    //
    //    visitConcreteModel(stitchedMdpUpper);
    //    const auto upperResult = currentPareto;
    //
    //    currentPareto = {lowerResult.first, upperResult.second};
    //    STORM_LOG_DEBUG("[COMPOSE] Exiting TraceModel");
}

template<typename ValueType>
std::pair<BidirectionalReachabilityResult<ValueType>, BidirectionalReachabilityResult<ValueType>> SymbioticTermination<ValueType>::getCurrentPareto() {
    return currentPareto;
}

template<typename ValueType>
std::unordered_map<std::string, storm::expressions::Expression> SymbioticTermination<ValueType>::getIdentifierMapping(
    storm::expressions::ExpressionManager const& manager) {
    std::unordered_map<std::string, storm::expressions::Expression> result;
    for (const auto& var : manager.getVariables()) {
        result[var.getName()] = var.getExpression();
    }
    return result;
}

template<typename ValueType>
std::unique_ptr<storm::modelchecker::CheckResult> performMultiObjectiveModelChecking(storm::Environment env, storm::models::sparse::Mdp<ValueType> const& mdp,
                                                                                     storm::logic::MultiObjectiveFormula const& formula) {
    typedef storm::models::sparse::Mdp<ValueType> SparseModelType;
    auto preprocessorResult =
        modelchecker::multiobjective::preprocessing::SparseMultiObjectivePreprocessor<storm::models::sparse::Mdp<ValueType>>::preprocess(env, mdp, formula);

    auto query = std::unique_ptr<storm::modelchecker::multiobjective::SparsePcaaQuery<SparseModelType, storm::RationalNumber>>(
        new storm::modelchecker::multiobjective::SparsePcaaParetoQuery<SparseModelType, storm::RationalNumber>(preprocessorResult));

    return query->check(env);
}

template class SymbioticTermination<double>;
template class SymbioticTermination<storm::RationalNumber>;

}  // namespace visitor
}  // namespace models
}  // namespace storm