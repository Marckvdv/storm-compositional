#include "storm-compose-cli/settings/modules/ComposeIOSettings.h"

#include "storm/settings/ArgumentBuilder.h"
#include "storm/settings/Option.h"
#include "storm/settings/OptionBuilder.h"
#include "storm/settings/SettingMemento.h"
#include "storm/settings/SettingsManager.h"

#include "storm/exceptions/InvalidArgumentException.h"

namespace storm {
namespace settings {
namespace modules {

const std::string ComposeIOSettings::moduleName = "compose";
const std::string ComposeIOSettings::stringDiagramOption = "stringdiagram";

ComposeIOSettings::ComposeIOSettings() : ModuleSettings(moduleName) {
    this->addOption(
        storm::settings::OptionBuilder(moduleName, stringDiagramOption, false, "model check the given string diagram")
            .addArgument(storm::settings::ArgumentBuilder::createStringArgument("filename", "The name of the file to model check (json).").build())
            .build());
}

bool ComposeIOSettings::check() const {
    return true;
}

void ComposeIOSettings::finalize() {
}

bool ComposeIOSettings::isStringDiagramSet() const {
    return this->getOption(stringDiagramOption).getHasOptionBeenSet();
}

std::string ComposeIOSettings::getStringDiagramFilename() const {
    return this->getOption(stringDiagramOption).getArgumentByName("filename").getValueAsString();
}

}  // namespace modules
}  // namespace settings
}  // namespace storm
