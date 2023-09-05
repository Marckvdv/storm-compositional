#include "storm/adapters/RationalNumberAdapter.h"
#include "storm/settings/modules/DebugSettings.h"
#include "storm/settings/modules/GeneralSettings.h"
#include "storm/analysis/GraphConditions.h"
#include "storm/api/storm.h"
#include "storm/modelchecker/results/ExplicitQualitativeCheckResult.h"
#include "storm/utility/initialize.h"
#include "storm/utility/NumberTraits.h"
#include "storm/utility/SignalHandler.h"
#include "storm/utility/Stopwatch.h"
#include "storm/exceptions/NotSupportedException.h"
#include "storm/exceptions/UnexpectedException.h"
#include "storm-cli-utilities/cli.h"
#include "storm-cli-utilities/model-handling.h"

#include "storm-compose-cli/settings/modules/ComposeIOSettings.h"
#include "storm-compose-cli/settings/ComposeSettings.h"
#include "storm-compose/parser/JsonStringDiagramParser.h"
#include "storm-compose/models/visitor/OpenMdpPrintVisitor.h"
#include "storm-compose/models/visitor/OpenMdpToDotVisitor.h"
#include "storm-compose/models/visitor/FlatMdpBuilderVisitor.h"
#include "storm-compose/models/visitor/ParetoVisitor.h"

#include "storm-parsers/parser/ExpressionParser.h"

#include <typeinfo>

namespace storm {
namespace compose {
namespace cli {

enum ReachabilityCheckingApproach {
    MONOLITHIC,
    NAIVE,
    WEIGHTED,
};

template <typename ValueType>
struct ReachabilityCheckingOptions {
    ReachabilityCheckingOptions() = default;

    std::shared_ptr<storm::models::OpenMdpManager<ValueType>> omdpManager;
    std::pair<bool, size_t> entrance {false, 0}, exit {true, 0};
    ReachabilityCheckingApproach approach = MONOLITHIC;
};

boost::optional<std::pair<bool, size_t>> parseEntranceExit(std::string text) {
    if (text.length() < 2) return boost::none;

    bool left = false;
    if (text[0] == 'l') left = true;
    else if (text[0] != 'r') return boost::none;

    return boost::optional<std::pair<bool, size_t>>({ left, std::stoi(text.substr(1)) });
}


template <typename ValueType>
boost::optional<ReachabilityCheckingOptions<ValueType>> processOptions() {
    ReachabilityCheckingOptions<ValueType> options;

    auto const& composeSettings = storm::settings::getModule<storm::settings::modules::ComposeIOSettings>();
    auto const& ioSettings = storm::settings::getModule<storm::settings::modules::IOSettings>();

    auto entrance = parseEntranceExit(composeSettings.getEntrance());
    auto exit = parseEntranceExit(composeSettings.getExit());
    if (!entrance || !exit) return boost::none;
    options.entrance = *entrance;
    options.exit = *exit;

    if (composeSettings.isApproachSet()) {
        std::string approach = composeSettings.getApproach();
        if (approach == "monolithic") options.approach = MONOLITHIC;
        else if (approach == "naive") options.approach = NAIVE;
        else if (approach == "weighted") options.approach = WEIGHTED;
        else STORM_LOG_THROW(false, storm::exceptions::NotSupportedException, approach << "is not supported");
    }

    std::string fileName = composeSettings.getStringDiagramFilename();
    STORM_PRINT_AND_LOG("Reading string diagram " << fileName << "\n");

    options.omdpManager = std::make_shared<storm::models::OpenMdpManager<ValueType>>();
    auto parser = storm::parser::JsonStringDiagramParser<ValueType>::fromFilePath(fileName, options.omdpManager);
    parser.parse();

    return options;
}

template <typename ValueType>
void performModelChecking(ReachabilityCheckingOptions<ValueType>& options) {
    switch (options.approach) {
        case MONOLITHIC:
            performMonolithicModelChecking(options);
            break;
        case NAIVE:
            performNaiveModelChecking(options);
            break;
        case WEIGHTED:
            performWeightedModelChecking(options);
            break;
    }
}

template <typename ValueType>
void performMonolithicModelChecking(ReachabilityCheckingOptions<ValueType>& options) {
    auto root = options.omdpManager->getRoot();
    options.omdpManager->constructConcreteMdps();
    storm::models::visitor::FlatMdpBuilderVisitor visitor(options.omdpManager);
    root->accept(visitor);

    auto mdp = visitor.getCurrent();
}

template <typename ValueType>
void performNaiveModelChecking(ReachabilityCheckingOptions<ValueType>& options) {
    auto root = options.omdpManager->getRoot();
    storm::models::visitor::ParetoVisitor visitor(options.omdpManager);
    root->accept(visitor);
}

template <typename ValueType>
void performWeightedModelChecking(ReachabilityCheckingOptions<ValueType>& options) {
    STORM_LOG_THROW(false, storm::exceptions::NotSupportedException, "currently not supported");
}

}  // namespace cli
}  // namespace compose
}  // namespace storm

/*!
 * Entry point for the compose backend.
 *
 * @param argc The argc argument of main().
 * @param argv The argv argument of main().
 * @return Return code, 0 if successfull, not 0 otherwise.
 */
int main(const int argc, const char** argv) {
    // try {
    storm::utility::setUp();
    storm::cli::printHeader("Storm-compose", argc, argv);
    storm::settings::initializeComposeSettings("Storm-compose", "storm-compose");

    bool optionsCorrect = storm::cli::parseOptions(argc, argv);
    if (!optionsCorrect) {
        return -1;
    }
    storm::utility::Stopwatch totalTimer(true);
    storm::cli::setUrgentOptions();

    // Invoke storm-compose with obtained settings
    auto const& generalSettings = storm::settings::getModule<storm::settings::modules::GeneralSettings>();

    if (generalSettings.isExactSet()) {
        auto options = storm::compose::cli::processOptions<storm::RationalNumber>();
        if (!options) {
            std::cout << "failed parsing options" << std::endl;
            return 1;
        }
        performModelChecking(*options);
    } else {
        auto options = storm::compose::cli::processOptions<double>();
        if (!options) {
            std::cout << "failed parsing options" << std::endl;
            return 1;
        }
        performModelChecking(*options);
    }

    totalTimer.stop();
    if (storm::settings::getModule<storm::settings::modules::ResourceSettings>().isPrintTimeAndMemorySet()) {
        storm::cli::printTimeAndMemoryStatistics(totalTimer.getTimeInMilliseconds());
    }

    // All operations have now been performed, so we clean up everything and terminate.
    storm::utility::cleanUp();
    return 0;
}
