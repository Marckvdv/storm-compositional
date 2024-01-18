#include "MonolithicOpenMdpChecker.h"

#include "io/DirectEncodingExporter.h"
#include "storm-compose/models/visitor/FlatMdpBuilderVisitor.h"
#include "storm-parsers/api/storm-parsers.h"
#include "storm-parsers/parser/FormulaParser.h"
#include "storm/environment/solver/MinMaxSolverEnvironment.h"
#include "storm/io/DirectEncodingExporter.h"
#include "storm/modelchecker/prctl/SparseMdpPrctlModelChecker.h"
#include "storm/modelchecker/results/ExplicitQuantitativeCheckResult.h"
#include "storm/solver/SolverSelectionOptions.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
MonolithicOpenMdpChecker<ValueType>::MonolithicOpenMdpChecker(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager,
                                                              storm::compose::benchmark::BenchmarkStats<ValueType>& stats)
    : AbstractOpenMdpChecker<ValueType>(manager, stats) {}

template<typename ValueType>
ApproximateReachabilityResult<ValueType> MonolithicOpenMdpChecker<ValueType>::check(OpenMdpReachabilityTask task) {
    this->stats.modelBuildingTime.start();
    this->manager->constructConcreteMdps();

    storm::models::visitor::FlatMdpBuilderVisitor<ValueType> flatVisitor(this->manager);
    this->manager->getRoot()->accept(flatVisitor);
    this->stats.modelBuildingTime.stop();

    auto concreteMdp = flatVisitor.getCurrent();

    auto& stateLabeling = concreteMdp.getMdp()->getStateLabeling();
    auto len0States = stateLabeling.getStates("len0");
    stateLabeling.setStates("init", len0States);

    STORM_LOG_WARN("Currently assuming len0 is the initial state");

    return checkConcreteMdp(concreteMdp, task);
}

template<typename ValueType>
ApproximateReachabilityResult<ValueType> MonolithicOpenMdpChecker<ValueType>::checkConcreteMdp(storm::models::ConcreteMdp<ValueType> const& concreteMdp,
                                                                                               OpenMdpReachabilityTask task) {
    auto mdp = concreteMdp.getMdp();

    if (false) {
        std::ofstream f("out.drn");
        storm::exporter::explicitExportSparseModel<ValueType>(f, mdp, {});
        f.close();
    }

    std::string formulaString = "Pmax=? [F ( \"" + task.getExitLabel() + "\" )]";
    storm::parser::FormulaParser formulaParser;
    auto formula = formulaParser.parseSingleFormulaFromString(formulaString);
    std::cout << "Formula: " << *formula << std::endl;

    storm::models::sparse::StateLabeling& labeling = mdp->getStateLabeling();
    if (!labeling.containsLabel("init")) {
        labeling.addLabel("init");
    }

    size_t entranceState = *labeling.getStates(task.getEntranceLabel()).begin();
    labeling.addLabelToState("init", entranceState);
    // std::cout << "Labeling of the MDP created: " << mdp->getStateLabeling() << std::endl;

    CheckTask<storm::logic::Formula, ValueType> checkTask(*formula, false);

    this->stats.reachabilityComputationTime.start();
    storm::modelchecker::SparseMdpPrctlModelChecker<storm::models::sparse::Mdp<ValueType>> checker(*mdp);

    ValueType precision = 1e-5;
    storm::Environment env;
    env.solver().minMax().setMethod(storm::solver::MinMaxMethod::OptimisticValueIteration);
    // env.solver().minMax().setMethod(storm::solver::MinMaxMethod::Topological);
    //  env.solver().minMax().setMethod(storm::solver::MinMaxMethod::PolicyIteration);
    env.solver().minMax().setPrecision(precision);

    auto sparseResult = checker.check(env, checkTask);
    this->stats.reachabilityComputationTime.stop();
    storm::modelchecker::ExplicitQuantitativeCheckResult<ValueType>& explicitResult = sparseResult->template asExplicitQuantitativeCheckResult<ValueType>();
    ValueType& lowerBound = explicitResult[entranceState];
    ValueType upperBound = storm::utility::min<ValueType>(lowerBound + precision, storm::utility::one<ValueType>());

    return ApproximateReachabilityResult<ValueType>(lowerBound, upperBound);
}

template class MonolithicOpenMdpChecker<storm::RationalNumber>;
template class MonolithicOpenMdpChecker<double>;

}  // namespace modelchecker
}  // namespace storm
