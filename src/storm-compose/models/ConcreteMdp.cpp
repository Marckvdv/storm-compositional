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

template<typename ValueType>
size_t ConcreteMdp<ValueType>::getEntranceCount() const {
    return lEntrance.size() + rEntrance.size();
}

template<typename ValueType>
size_t ConcreteMdp<ValueType>::getExitCount() const {
    return lExit.size() + rExit.size();
}

template class ConcreteMdp<double>;
template class ConcreteMdp<storm::RationalNumber>;

}  // namespace models
}  // namespace storm
