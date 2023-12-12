#include "CompositionalValueIteration.h"
#include "storm-compose/modelchecker/CompositionalValueVector.h"
#include "storm-compose/storage/EntranceExit.h"
#include "storm-compose/models/visitor/ParetoInitializerVisitor.h"
#include "storm-compose/models/visitor/PropertyDrivenVisitor.h"
#include "storm-compose/models/visitor/MappingVisitor.h"
#include "storm-compose/models/visitor/EntranceExitVisitor.h"
#include "storm-compose/models/visitor/OuterEntranceExitVisitor.h"
#include "storm-compose/models/visitor/EntranceExitMappingVisitor.h"
#include "storm-compose/storage/ExactCache.h"
#include "storm-compose/storage/NoCache.h"
#include "storm-compose/storage/ParetoCache.h"
#include "storm/utility/SignalHandler.h"
#include "storm/utility/constants.h"
#include "storm/exceptions/NotSupportedException.h"
#include "storm/exceptions/InvalidOperationException.h"

namespace storm {
namespace modelchecker {

template<typename ValueType>
CompositionalValueIteration<ValueType>::CompositionalValueIteration(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager,
                                                                    storm::compose::benchmark::BenchmarkStats<ValueType>& stats, Options options)
    : AbstractOpenMdpChecker<ValueType>(manager, stats), options(options) {}

template<typename ValueType>
void CompositionalValueIteration<ValueType>::initializeParetoCurves() {
    models::visitor::ParetoInitializerVisitor<ValueType> visitor;
    this->manager->getRoot()->accept(visitor);
}

/*
** Current algorithm structure
** Store 1 global value vector, which indicates the (lower bound of the) optimal value for each input.
** Optimal in the sense of optimizing the (global) input weight.
** Due to the way sum and sequential composition work, many ouputs are also inputs.
**
** In fact, we require that all entrances and exits are either:
** a) an entrance or exit at the root of the string diagram
** b) connected to some other entrance/exit.
**
** We need a mapping from each component's entrance to this value vector.
** Such a mapping maps <component id, entrance id, left entrance> -> size_t
**
** Once we have such a mapping, the model checking procedure goes as follows:
** 1) Initialize the value vector to all zeroes.
** 2) Need to start at the edge of the string diagram as these are the only positions with non-zero values (due to the intial weight vector).
** 3) Given a leaf node, perform standard value iteration with initial weights.
** If there are exits at the edge, the weight is taken from the initial weight.
** For exits that are intermediate, the weight is taken from the global value vector.
** We obtain optimal values for the entrances, which are stored back into the global value vector.
** 4) Repeat. Termination either using the 'optimistic VI' approach or bottom-up TACAS style
 */
template<typename ValueType>
ApproximateReachabilityResult<ValueType> CompositionalValueIteration<ValueType>::check(OpenMdpReachabilityTask task) {
    initialize(task);

    std::cout << "initial values:" << storm::utility::convertNumber<double>(valueVector.getValues()[0]) << "( " << currentStep << "/" << options.maxSteps << " )"<< std::endl;
    for (const auto &v : valueVector.getValues()) {
        std::cout << " " << storm::utility::convertNumber<double>(v);
    }
    std::cout << std::endl;

    auto noCache = std::make_shared<storage::NoCache<ValueType>>();
    auto root = this->manager->getRoot();
    do {
        models::visitor::CVIVisitor<ValueType> cviVisitor(this->manager, valueVector, cache);
        root->accept(cviVisitor);

        std::cout << "current values:" << storm::utility::convertNumber<double>(valueVector.getValues()[0]) << "( " << currentStep << "/" << options.maxSteps << " )"<< std::endl;
        for (const auto &v : valueVector.getValues()) {
            std::cout << " " << storm::utility::convertNumber<double>(v);
        }
        std::cout << std::endl;

        // Compute v + epsilon
        auto newValue = valueVector;
        newValue.addConstant(options.epsilon);
        auto newValueCopy = newValue;

        //models::visitor::CVIVisitor<ValueType> upperboundVisitor(this->manager, newValue, noCache);
        models::visitor::CVIVisitor<ValueType> upperboundVisitor(this->manager, newValue, cache);
        root->accept(upperboundVisitor);
        if (newValueCopy.dominates(newValue)) {
            // optimistic value iteration stopping criterion
            std::cout << "OVI stopping criterion hit" << std::endl;
            break;
        }

        ++currentStep;
    } while(!shouldTerminate());


    std::cout << "WARN for now assuming we are only interested in the first left entrance" << std::endl;
    std::cout << "WARN assuming first entrance is the one of interest" << std::endl;

    auto lowerBound = valueVector.getValues()[0];
    auto upperBound = utility::min<ValueType>(lowerBound + options.epsilon, utility::one<ValueType>());
    ApproximateReachabilityResult<ValueType> result(lowerBound, upperBound);
    return result;
}

template<typename ValueType>
void CompositionalValueIteration<ValueType>::initialize(OpenMdpReachabilityTask const& task) {
    initializeParetoCurves();
    initializeCache();

    this->manager->constructConcreteMdps();
    auto root = this->manager->getRoot();

    models::visitor::MappingVisitor<ValueType> mappingVisitor;
    root->accept(mappingVisitor);
    auto mapping = mappingVisitor.getMapping();
    mapping.print();

    models::visitor::PropertyDrivenVisitor visitor(this->manager);

    models::visitor::EntranceExitMappingVisitor<ValueType> entranceExitMappingVisitor;
    root->accept(entranceExitMappingVisitor);

    // TODO FIXME can be retrieved from mapping visitor above
    models::visitor::EntranceExitVisitor<ValueType> entranceExitVisitor;
    entranceExitVisitor.setEntranceExit(storage::L_EXIT);
    root->accept(entranceExitVisitor);
    size_t lOuterExitCount = entranceExitVisitor.getCollected().size();

    entranceExitVisitor.setEntranceExit(storage::R_EXIT);
    root->accept(entranceExitVisitor);
    size_t rOuterExitCount = entranceExitVisitor.getCollected().size();

    auto finalWeight = task.toExitWeights<ValueType>(lOuterExitCount, rOuterExitCount);
    valueVector = models::visitor::ValueVector<ValueType>(mappingVisitor.getMapping(), finalWeight);
    valueVector.initializeValues();
}

template<typename ValueType>
void CompositionalValueIteration<ValueType>::initializeCache() {
    cache = std::make_shared<storm::storage::ExactCache<ValueType>>();
    //cache = std::make_shared<storm::storage::ParetoCache<ValueType>>();
}

template<typename ValueType>
bool CompositionalValueIteration<ValueType>::shouldTerminate() {
    return currentStep > options.maxSteps || storm::utility::resources::isTerminate();
}

template class CompositionalValueIteration<storm::RationalNumber>;
template class CompositionalValueIteration<double>;

}  // namespace modelchecker
}  // namespace storm
