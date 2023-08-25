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

template<typename ValueType>
void performModelChecking() {
    auto const& composeSettings = storm::settings::getModule<storm::settings::modules::ComposeIOSettings>();
    auto const& ioSettings = storm::settings::getModule<storm::settings::modules::IOSettings>();

    std::string fileName = composeSettings.getStringDiagramFilename();
    STORM_PRINT_AND_LOG("Reading string diagram " << fileName << "\n");

    auto omdpManager = std::make_shared<storm::models::OpenMdpManager<ValueType>>();
    auto parser = storm::parser::JsonStringDiagramParser<ValueType>::fromFilePath(fileName, omdpManager);
    parser.parse();

    auto root = omdpManager->getRoot();

    storm::models::visitor::ParetoVisitor<ValueType> paretoVisitor(omdpManager);
    root->accept(paretoVisitor);

    //auto concurrentMdp = paretoVisitor.getCurrent();
    //auto mdp = concurrentMdp.getMdp();

    //if (ioSettings.isExportDotSet()) {
    //    std::ofstream dotOutFile;
    //    std::string name = ioSettings.getExportDotFilename();
    //    utility::openFile(name, dotOutFile);

    //    mdp->writeDotToStream(dotOutFile);
    //}

    omdpManager->constructConcreteMdps();

    /*
    if (ioSettings.isExportDotSet()) {
        std::ofstream dotOutFile;
        std::string name = ioSettings.getExportDotFilename();
        utility::openFile(name, dotOutFile);

        storm::models::visitor::OpenMdpToDotVisitor<ValueType> printDot(dotOutFile);
        printDot.visitRoot(*root);
    }
    */

    storm::models::visitor::FlatMdpBuilderVisitor<ValueType> flatBuilder(omdpManager);
    root->accept(flatBuilder);

    auto flatMdp = flatBuilder.getCurrent();

    if (ioSettings.isExportDotSet()) {
        std::ofstream dotOutFile;
        std::string name = "flat_" + ioSettings.getExportDotFilename();
        utility::openFile(name, dotOutFile);

        flatMdp.getMdp()->writeDotToStream(dotOutFile);
    }
}

void processOptions() {
    auto const& composeSettings = storm::settings::getModule<storm::settings::modules::ComposeIOSettings>();
    auto const& generalSettings = storm::settings::getModule<storm::settings::modules::GeneralSettings>();

    if (!composeSettings.isStringDiagramSet()) {
        STORM_PRINT_AND_LOG("No (string diagram) input model given\n");
        return;
    }

    if (generalSettings.isExactSet()) {
        performModelChecking<storm::RationalNumber>();
    } else {
        performModelChecking<double>();
    }
}


}  // namespace cli
}  // namespace compose
}  // namespace storm

/*!
 * Entry point for the pomdp backend.
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
    storm::compose::cli::processOptions();

    totalTimer.stop();
    if (storm::settings::getModule<storm::settings::modules::ResourceSettings>().isPrintTimeAndMemorySet()) {
        storm::cli::printTimeAndMemoryStatistics(totalTimer.getTimeInMilliseconds());
    }

    // All operations have now been performed, so we clean up everything and terminate.
    storm::utility::cleanUp();
    return 0;
    // } catch (storm::exceptions::BaseException const &exception) {
    //    STORM_LOG_ERROR("An exception caused Storm-pomdp to terminate. The message of the exception is: " << exception.what());
    //    return 1;
    //} catch (std::exception const &exception) {
    //    STORM_LOG_ERROR("An unexpected exception occurred and caused Storm-pomdp to terminate. The message of this exception is: " << exception.what());
    //    return 2;
    //}
}
