#include "MonolithicOpenMdpChecker.h"

#include "solver/SolverSelectionOptions.h"
#include "storm-compose/models/visitor/FlatMdpBuilderVisitor.h"
#include "storm-parsers/api/storm-parsers.h"
#include "storm-parsers/parser/FormulaParser.h"
#include "storm/environment/Environment.h"
#include "storm/modelchecker/prctl/SparseMdpPrctlModelChecker.h"
#include "storm/modelchecker/results/ExplicitQuantitativeCheckResult.h"
#include "storm/solver/SolverSelectionOptions.h"
#include "storm/environment/solver/MinMaxSolverEnvironment.h"

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
    auto mdp = concreteMdp.getMdp();

    // bool exportToDot = true;
    // if (exportToDot) {
    //     std::ofstream out("test.dot");
    //     mdp->writeDotToStream(out);
    // }

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
    std::cout << "Labeling of the MDP created: " << mdp->getStateLabeling() << std::endl;

    CheckTask<storm::logic::Formula, ValueType> checkTask(*formula, false);

    this->stats.reachabilityComputationTime.start();
    storm::modelchecker::SparseMdpPrctlModelChecker<storm::models::sparse::Mdp<ValueType>> checker(*mdp);
    storm::Environment env;
    env.solver().minMax().setMethod(storm::solver::MinMaxMethod::OptimisticValueIteration);
    auto sparseResult = checker.check(env, checkTask);
    this->stats.reachabilityComputationTime.stop();
    storm::modelchecker::ExplicitQuantitativeCheckResult<ValueType>& explicitResult = sparseResult->template asExplicitQuantitativeCheckResult<ValueType>();
    ValueType& exactValue = explicitResult[entranceState];

    return ApproximateReachabilityResult<ValueType>(exactValue);
}

template class MonolithicOpenMdpChecker<storm::RationalNumber>;
template class MonolithicOpenMdpChecker<double>;

}  // namespace modelchecker
}  // namespace storm
