#include "PropertyDrivenVisitor.h"

#include "storm-compose/models/visitor/ParetoVisitor.h"
#include "storm-parsers/parser/FormulaParser.h"
#include "storm/modelchecker/multiobjective/pcaa/StandardMdpPcaaWeightVectorChecker.h"
#include "storm/modelchecker/multiobjective/preprocessing/SparseMultiObjectivePreprocessor.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
PropertyDrivenVisitor<ValueType>::PropertyDrivenVisitor(std::shared_ptr<OpenMdpManager<ValueType>> manager) : manager(manager) {}

template<typename ValueType>
PropertyDrivenVisitor<ValueType>::~PropertyDrivenVisitor() {}

template<typename ValueType>
void PropertyDrivenVisitor<ValueType>::visitPrismModel(PrismModel<ValueType>& model) {
    STORM_LOG_ASSERT(false, "Expected concrete models");
}

template<typename ValueType>
void PropertyDrivenVisitor<ValueType>::visitConcreteModel(ConcreteMdp<ValueType>& model) {
    currentWeight = weightedReachability(currentWeight, model);
}

template<typename ValueType>
void PropertyDrivenVisitor<ValueType>::visitReference(Reference<ValueType>& reference) {
    // We could do some caching here, but the problem is that this depends on the weight used.
    // I suspect the amount of actual caching to be done is rather low.
    const auto& referenceName = reference.getReference();
    const auto manager = reference.getManager();
    auto dereferenced = manager->dereference(referenceName);
    dereferenced->accept(*this);
}

template<typename ValueType>
void PropertyDrivenVisitor<ValueType>::visitSequenceModel(SequenceModel<ValueType>& model) {
    // Iterate from right to left
    // TODO replace this confusing construct below
    for (size_t i = model.getValues().size(); i-- > 0;) {
        model.getValues()[i]->accept(*this);
    }
}

template<typename ValueType>
void PropertyDrivenVisitor<ValueType>::visitSumModel(SumModel<ValueType>& model) {
    WeightType originalWeight = currentWeight;
    WeightType finalWeight;

    typename storm::models::OpenMdp<ValueType>::Scope emptyScope;
    // We need to split the weights for each value
    size_t index = 0;
    for (auto& value : model.getValues()) {
        std::vector<ValueType> weight;
        auto exits = value->collectEntranceExit(OpenMdp<ValueType>::R_EXIT, emptyScope);

        std::cout << "Exits size: " << exits.size() << std::endl;

        bool allZero = true;
        for (size_t i = 0; i < exits.size(); ++i) {
            ValueType& val = originalWeight[index];
            if (!storm::utility::isAlmostZero(val)) {  // TODO check definition of isAlmostZero
                allZero = false;
            }

            weight.push_back(val);
            ++index;
        }

        std::cout << "Weight size: " << weight.size() << std::endl;

        if (allZero) {
            std::cout << "SKIP" << std::endl;
            auto entrances = value->collectEntranceExit(OpenMdp<ValueType>::L_ENTRANCE, emptyScope);
            for (size_t i = 0; i < entrances.size(); ++i) {
                finalWeight.push_back(storm::utility::zero<ValueType>());
            }
        } else {
            currentWeight = weight;
            value->accept(*this);
            finalWeight.insert(finalWeight.end(), currentWeight.begin(), currentWeight.end());
        }
    }

    currentWeight = finalWeight;
}

template<typename ValueType>
void PropertyDrivenVisitor<ValueType>::visitTraceModel(TraceModel<ValueType>& model) {
    STORM_LOG_ASSERT(false, "currently not implemented!");
}

template<typename ValueType>
void PropertyDrivenVisitor<ValueType>::setWeight(std::vector<ValueType> weight) {
    currentWeight = std::move(weight);
}

template<typename ValueType>
void PropertyDrivenVisitor<ValueType>::setTargetExit(size_t exitCount, size_t exit, bool leftExit) {
    STORM_LOG_ASSERT(!leftExit, "Only right exits allowed at the moment");

    std::vector<ValueType> weights(exitCount);
    weights[exit] = storm::utility::one<ValueType>();

    setWeight(weights);
}

template<typename ValueType>
std::vector<ValueType> PropertyDrivenVisitor<ValueType>::weightedReachability(std::vector<ValueType> weights, ConcreteMdp<ValueType> concreteMdp) {
    using storm::modelchecker::multiobjective::StandardMdpPcaaWeightVectorChecker;
    using storm::modelchecker::multiobjective::preprocessing::SparseMultiObjectivePreprocessor;
    using storm::models::sparse::Mdp;

    setWeight(weights);
    std::string formulaString = ParetoVisitor<ValueType>::getFormula(concreteMdp);

    storm::parser::FormulaParser formulaParser;
    auto formula = formulaParser.parseSingleFormulaFromString(formulaString);
    std::cout << "Formula: " << formulaString << std::endl;
    std::cout << "with weights: " << std::endl;
    for (const auto& v : weights) {
        std::cout << v << std::endl;
    }

    auto mdp = concreteMdp.getMdp();

    std::vector<ValueType> newWeights;
    for (size_t entrance : concreteMdp.getLEntrance()) {
        // TODO make efficient
        mdp->getStateLabeling().setStates("init", storage::BitVector(mdp->getNumberOfStates()));
        mdp->getStateLabeling().addLabelToState("init", entrance);

        auto preprocessResult = SparseMultiObjectivePreprocessor<Mdp<ValueType>>::preprocess(this->env, *mdp, formula->asMultiObjectiveFormula());
        StandardMdpPcaaWeightVectorChecker checker(preprocessResult);

        checker.check(this->env, weights);
        auto underApprox = checker.getUnderApproximationOfInitialStateResults();

        std::cout << "intermediate result for entrance state " << entrance << ":" << std::endl;
        ValueType sum = 0;
        for (size_t i = 0; i < weights.size(); ++i) {
            sum += weights[i] * underApprox[i];
            std::cout << underApprox[i] << std::endl;
        }

        newWeights.push_back(sum);
    }

    return newWeights;
}

template<typename ValueType>
std::vector<ValueType> PropertyDrivenVisitor<ValueType>::getCurrentWeight() {
    return currentWeight;
}

template class PropertyDrivenVisitor<storm::RationalNumber>;
template class PropertyDrivenVisitor<double>;

}  // namespace visitor
}  // namespace models
}  // namespace storm
