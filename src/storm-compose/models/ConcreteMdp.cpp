#include "ConcreteMdp.h"

#include "storm-compose/models/visitor/BidirectionalReachabilityResult.h"
#include "storm/environment/Environment.h"
#include "storm/modelchecker/multiobjective/Objective.h"
#include "storm/modelchecker/multiobjective/pcaa/StandardMdpPcaaWeightVectorChecker.h"
#include "storm/modelchecker/multiobjective/preprocessing/SparseMultiObjectivePreprocessor.h"

namespace storm {
namespace models {

template<typename ValueType>
ConcreteMdp<ValueType>::ConcreteMdp(std::weak_ptr<OpenMdpManager<ValueType>> manager) : OpenMdp<ValueType>(manager), mdp(nullptr) {}

template<typename ValueType>
ConcreteMdp<ValueType>::ConcreteMdp(std::weak_ptr<OpenMdpManager<ValueType>> manager, std::shared_ptr<storm::models::sparse::Mdp<ValueType>> mdp,
                                    std::vector<size_t> lEntrance, std::vector<size_t> rEntrance, std::vector<size_t> lExit, std::vector<size_t> rExit)
    : OpenMdp<ValueType>(manager), mdp(mdp), lEntrance(lEntrance), rEntrance(rEntrance), lExit(lExit), rExit(rExit) {}

template<typename ValueType>
ConcreteMdp<ValueType>::~ConcreteMdp() {}

template<typename ValueType>
bool ConcreteMdp<ValueType>::isConcreteMdp() const {
    return true;
}

template<typename ValueType>
void ConcreteMdp<ValueType>::accept(visitor::OpenMdpVisitor<ValueType>& visitor) {
    visitor.visitConcreteModel(*this);
}

template<typename ValueType>
std::shared_ptr<storm::models::sparse::Mdp<ValueType>> ConcreteMdp<ValueType>::getMdp() {
    return mdp;
}

template<typename ValueType>
const std::shared_ptr<storm::models::sparse::Mdp<ValueType>> ConcreteMdp<ValueType>::getMdp() const {
    return mdp;
}

template<typename ValueType>
std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> ConcreteMdp<ValueType>::collectEntranceExit(
    typename OpenMdp<ValueType>::EntranceExit entranceExit, typename OpenMdp<ValueType>::Scope& scope) const {
    std::vector<size_t> const* src = nullptr;

    if (entranceExit == OpenMdp<ValueType>::L_ENTRANCE)
        src = &lEntrance;
    else if (entranceExit == OpenMdp<ValueType>::R_ENTRANCE)
        src = &rEntrance;
    else if (entranceExit == OpenMdp<ValueType>::L_EXIT)
        src = &lExit;
    else if (entranceExit == OpenMdp<ValueType>::R_EXIT)
        src = &rExit;

    // Doing some pointer magic, double check
    std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> entries;
    size_t i = 0;
    for (auto entry : *src) {
        scope.pushScope(i);
        typename OpenMdp<ValueType>::ConcreteEntranceExit newEntry{this, entry, scope};
        scope.popScope();
        entries.push_back(newEntry);
        ++i;
    }
    return entries;
}

template<typename ValueType>
void ConcreteMdp<ValueType>::exportToDot(std::string path) {
    std::ofstream outputFile;
    outputFile.open(path);
    mdp->writeDotToStream(outputFile);
}

template<typename ValueType>
bool ConcreteMdp<ValueType>::isRightward() const {
    return rEntrance.size() == 0 && lExit.size() == 0;
}

template<typename ValueType>
std::vector<ValueType> ConcreteMdp<ValueType>::weightedReachability(std::vector<ValueType> const& weight) {
    // Step 1.1) Construct objectives for each of the exits.
    visitor::BidirectionalReachabilityResult<ValueType> currentResults;
    auto objectives = currentResults.getReachabilityObjectives();
    std::vector<std::shared_ptr<storm::logic::Formula const>> formulas;
    for (auto& objectives : objectives) {
        formulas.push_back(objectives.originalFormula);
    }

    // Step 2) perform weighted reachability on the model
    storm::Environment env;

    typedef storm::models::sparse::Mdp<ValueType> SparseModelType;
    auto preprocessResult = modelchecker::multiobjective::preprocessing::SparseMultiObjectivePreprocessor<SparseModelType>::preprocess(env, *mdp, formulas);
    storm::modelchecker::multiobjective::StandardMdpPcaaWeightVectorChecker<SparseModelType> checker(preprocessResult);

    checker.check(env, weight);

    // Step 3) obtain reachability results for initial states (= entrances)
    auto results = checker.getUnderApproximationOfInitialStateResults();
}

template<typename ValueType>
std::vector<size_t> const& ConcreteMdp<ValueType>::getLEntrance() const {
    return lEntrance;
}

template<typename ValueType>
std::vector<size_t> const& ConcreteMdp<ValueType>::getREntrance() const {
    return rEntrance;
}

template<typename ValueType>
std::vector<size_t> const& ConcreteMdp<ValueType>::getLExit() const {
    return lExit;
}

template<typename ValueType>
std::vector<size_t> const& ConcreteMdp<ValueType>::getRExit() const {
    return rExit;
}

template class ConcreteMdp<double>;
template class ConcreteMdp<storm::RationalNumber>;

}  // namespace models
}  // namespace storm
