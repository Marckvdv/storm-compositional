#include "ConcreteMdp.h"

namespace storm {
namespace models {

template<typename ValueType>
ConcreteMdp<ValueType>::ConcreteMdp(std::weak_ptr<OpenMdpManager<ValueType>> manager)
    : OpenMdp<ValueType>(manager), mdp(nullptr) {
}

template<typename ValueType>
ConcreteMdp<ValueType>::ConcreteMdp(std::weak_ptr<OpenMdpManager<ValueType>> manager, std::shared_ptr<storm::models::sparse::Mdp<ValueType>> mdp, std::vector<size_t> lEntrance, std::vector<size_t> rEntrance, std::vector<size_t> lExit, std::vector<size_t> rExit)
    : OpenMdp<ValueType>(manager), mdp(mdp), lEntrance(lEntrance), rEntrance(rEntrance), lExit(lExit), rExit(rExit) {
}

template<typename ValueType>
ConcreteMdp<ValueType>::~ConcreteMdp() {
}

template<typename ValueType>
bool ConcreteMdp<ValueType>::isConcreteMdp() const {
    return true;
}

template <typename ValueType>
void ConcreteMdp<ValueType>::accept(visitor::OpenMdpVisitor<ValueType>& visitor) {
    visitor.visitConcreteModel(*this);
}

template <typename ValueType>
std::shared_ptr<storm::models::sparse::Mdp<ValueType>> ConcreteMdp<ValueType>::getMdp() {
    return mdp;
}

template <typename ValueType>
const std::shared_ptr<storm::models::sparse::Mdp<ValueType>> ConcreteMdp<ValueType>::getMdp() const {
    return mdp;
}

template <typename ValueType>
std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> ConcreteMdp<ValueType>::collectEntranceExit(typename OpenMdp<ValueType>::EntranceExit entranceExit, typename OpenMdp<ValueType>::Scope& scope) const {
    std::vector<size_t> const* src = nullptr;

    if (entranceExit == OpenMdp<ValueType>::L_ENTRANCE) src = &lEntrance;
    else if (entranceExit == OpenMdp<ValueType>::R_ENTRANCE) src = &rEntrance;
    else if (entranceExit == OpenMdp<ValueType>::L_EXIT) src = &lExit;
    else if (entranceExit == OpenMdp<ValueType>::R_EXIT) src = &rExit;

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

template <typename ValueType>
std::vector<ValueType> ConcreteMdp<ValueType>::weightedReachability(std::vector<ValueType> const& weights) {
    //if (!isPreparedForWeightedReachability) {
    //    prepareForWeightedReachability();
    //}
}

template <typename ValueType>
void ConcreteMdp<ValueType>::prepareForWeightedReachability() {
    // We prepare this concrete MDP for weighted reachability by doing the following.

    isPreparedForWeightedReachability = true;
}

template <typename ValueType>
void ConcreteMdp<ValueType>::setWeights(std::vector<ValueType> const& weights) {

}

template class ConcreteMdp<double>;
template class ConcreteMdp<storm::RationalNumber>;

}
}
