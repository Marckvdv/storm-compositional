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
const std::string ComposeIOSettings::entranceName = "entrance";
const std::string ComposeIOSettings::exitName = "exit";
const std::string ComposeIOSettings::approachName = "approach";
const std::string ComposeIOSettings::exportStringDiagramName = "exportStringDiagram";

ComposeIOSettings::ComposeIOSettings() : ModuleSettings(moduleName) {
    this->addOption(
        storm::settings::OptionBuilder(moduleName, stringDiagramOption, false, "model check the given string diagram")
            .addArgument(storm::settings::ArgumentBuilder::createStringArgument("filename", "The name of the file to model check (json).").build())
            .build());
    this->addOption(
        storm::settings::OptionBuilder(moduleName, entranceName, false, "entrance to consider as the initial state of the string diagram")
            .addArgument(storm::settings::ArgumentBuilder::createStringArgument("entrance", "<l|r><number> e.g. l5 is left entrance 5, default: l0").build())
            .build());
    this->addOption(
        storm::settings::OptionBuilder(moduleName, exitName, false, "exit to consider as the target state of the string diagram")
            .addArgument(storm::settings::ArgumentBuilder::createStringArgument("exit", "<l|r><number> e.g. r3 is right exit 3, default: r0").build())
            .build());
    this->addOption(
        storm::settings::OptionBuilder(moduleName, approachName, false, "approach to use for computing reachability in the string diagram")
            .addArgument(storm::settings::ArgumentBuilder::createStringArgument("approach", "(choose from {monolithic, naive, weight}, default: monolithic").build())
            .build());
    this->addOption(
        storm::settings::OptionBuilder(moduleName, exportStringDiagramName, false, "export the string diagram to a dot file")
            .addArgument(storm::settings::ArgumentBuilder::createStringArgument("filename", "The name of the file to write the dot file").build())
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

bool ComposeIOSettings::isEntranceSet() const {
    return this->getOption(entranceName).getHasOptionBeenSet();
}

bool ComposeIOSettings::isExitSet() const {
    return this->getOption(exitName).getHasOptionBeenSet();
}

bool ComposeIOSettings::isApproachSet() const {
    return this->getOption(approachName).getHasOptionBeenSet();
}

bool ComposeIOSettings::isExportStringDiagramSet() const {
    return this->getOption(exportStringDiagramName).getHasOptionBeenSet();
}

std::string ComposeIOSettings::getStringDiagramFilename() const {
    return this->getOption(stringDiagramOption).getArgumentByName("filename").getValueAsString();
}

std::string ComposeIOSettings::getEntrance() const {
    if (isEntranceSet()) {
        return this->getOption(entranceName).getArgumentByName("entrance").getValueAsString();
    } else {
        return "l0";
    }
}

std::string ComposeIOSettings::getExit() const {
    if (isExitSet()) {
        return this->getOption(exitName).getArgumentByName("exit").getValueAsString();
    } else {
        return "r0";
    }
}

std::string ComposeIOSettings::getApproach() const {
    if (isApproachSet()) {
        return this->getOption(approachName).getArgumentByName("approach").getValueAsString();
    } else {
        return "monolithic";
    }
}

std::string ComposeIOSettings::getExportStringDiagramFilename() const {
    return this->getOption(exportStringDiagramName).getArgumentByName("filename").getValueAsString();
}

}  // namespace modules
}  // namespace settings
}  // namespace storm
