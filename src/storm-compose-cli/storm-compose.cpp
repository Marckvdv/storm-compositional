#include "storm-compose-cli/stringdiagram/StringDiagramModelChecking.h"
#include "storm-compose-cli/dfa/DfaModelChecking.h"

#include <typeinfo>

namespace storm {
namespace compose {
namespace cli {

template<typename T>
void processGeneric() {
    auto& composeSettings = storm::settings::getModule<storm::settings::modules::ComposeIOSettings>();
    bool compatible = true;
    compatible &= !(composeSettings.isDfaPropSet() && composeSettings.isStringDiagramSet());

    if (!compatible) {
        throw exceptions::UnexpectedException("incompatible combination of options");
    }

    if (composeSettings.isStringDiagramSet()) {
        auto options = storm::compose::cli::processStringDiagramOptions<T>();
        if (!options) {
            std::cout << "failed parsing options" << std::endl;
            storm::compose::cli::performStringDiagramModelChecking<T>(*options);
        }
    } else if (composeSettings.isDfaPropSet()) {
        performDfaModelChecking<T>();
    }
}

void process() {
    auto const& generalSettings = storm::settings::getModule<storm::settings::modules::GeneralSettings>();
    if (generalSettings.isExactSet()) {
        processGeneric<storm::RationalNumber>();
    } else {
        processGeneric<double>();
    }
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
    return storm::cli::process("Storm-compose", "storm-compose", storm::settings::initializeComposeSettings, storm::compose::cli::process, argc, argv);
}
