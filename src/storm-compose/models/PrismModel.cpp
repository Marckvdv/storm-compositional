#include "PrismModel.h"
#include "storm/builder/ExplicitModelBuilder.h"
#include "storm/generator/PrismNextStateGenerator.h"
#include "storm-parsers/api/storm-parsers.h"
#include "storm-compose/parser/StateValuationParser.h"
#include <memory>

namespace storm {
namespace models {

template<typename ValueType>
PrismModel<ValueType>::PrismModel(std::weak_ptr<OpenMdpManager<ValueType>> manager, std::string path,
               std::vector<std::string> lEntrance, std::vector<std::string> rEntrance,
               std::vector<std::string> lExit, std::vector<std::string> rExit
) : OpenMdp<ValueType>(manager), path(path), lEntrance(lEntrance), rEntrance(rEntrance), lExit(lExit), rExit(rExit) {
}

template<typename ValueType>
PrismModel<ValueType>::~PrismModel() {
}

template<typename ValueType>
bool PrismModel<ValueType>::isPrismModel() const {
    return true;
}

template<typename ValueType>
std::string PrismModel<ValueType>::getPath() const {
    return path;
}

template <typename ValueType>
void PrismModel<ValueType>::accept(visitor::OpenMdpVisitor<ValueType>& visitor) {
    visitor.visitPrismModel(*this);
}

template <typename ValueType>
ConcreteMdp<ValueType> PrismModel<ValueType>::toConcreteMdp() {
    // 1) Load the PRISM file
    storm::builder::BuilderOptions buildOptions;
    buildOptions.setBuildStateValuations();
    buildOptions.setBuildChoiceLabels();
    buildOptions.setBuildAllLabels();

    auto program = storm::api::parseProgram(getPath(), false, false);
    auto generator = std::make_shared<storm::generator::PrismNextStateGenerator<ValueType, uint32_t>>(program, buildOptions);
    storm::builder::ExplicitModelBuilder<ValueType> mdpBuilder(generator);

    auto mdp = std::dynamic_pointer_cast<storm::models::sparse::Mdp<ValueType>>(mdpBuilder.build());
    auto& labeling = mdp->getStateLabeling();

    // 2) resolve entrances and exits
    const auto& stateValuations = mdp->getStateValuations();
    // TODO make efficient

    storm::parser::StateValuationParser valuationParser(stateValuations);
    std::vector<size_t> lEntranceIdx, rEntranceIdx, lExitIdx, rExitIdx;

    auto processEntranceExit = [&](auto& source, auto& dest, bool entrance, bool left) {
        size_t count = 0;
        for (auto& entry : source) {
            auto stateVal = valuationParser.parseStateValuation(entry);
            auto optionalIndex = stateValuations.findState(stateVal);
            if (optionalIndex) {
                dest.push_back(*optionalIndex);
                std::string label = (std::string() + (left ? "l" : "r")) + (entrance ? "en" : "ex") + std::to_string(count);
                labeling.addLabel(label);
                labeling.addLabelToState(label, *optionalIndex);
            } else {
                STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "Could not find state valuation " << entry << " in prism file " << getPath());
            }
            ++count;
        }
    };
    processEntranceExit(lEntrance, lEntranceIdx, true, true);
    processEntranceExit(rEntrance, rEntranceIdx, true, false);
    processEntranceExit(lExit, lExitIdx, false, true);
    processEntranceExit(rExit, rExitIdx, false, false);

    return ConcreteMdp<ValueType>(this->manager, mdp, lEntranceIdx, rEntranceIdx, lExitIdx, rExitIdx);
}

template <typename ValueType>
bool PrismModel<ValueType>::isRightward() const {
    return rEntrance.size() == 0 && lExit.size() == 0;
}

template <typename ValueType>
std::vector<typename OpenMdp<ValueType>::ConcreteEntranceExit> PrismModel<ValueType>::collectEntranceExit(typename OpenMdp<ValueType>::EntranceExit entryExit, typename OpenMdp<ValueType>::Scope& scope) const {
    STORM_LOG_ASSERT(false, "Concretize all OpenMdps before calling this function");
    return {};
}

template class PrismModel<storm::RationalNumber>;
template class PrismModel<double>;

}
}
