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
const std::string ComposeIOSettings::benchmarkDataName = "benchmarkData";
const std::string ComposeIOSettings::paretoPrecisionName = "paretoPrecision";
const std::string ComposeIOSettings::paretoPrecisionTypeName = "paretoPrecisionType";
const std::string ComposeIOSettings::paretoStepsName = "paretoSteps";
const std::string ComposeIOSettings::oviEpsilonName = "oviEpsilon";

ComposeIOSettings::ComposeIOSettings() : ModuleSettings(moduleName) {
    auto addStringOption = [&](std::string optionName, std::string description, std::string fieldName, std::string fieldDescription) {
        this->addOption(storm::settings::OptionBuilder(moduleName, optionName, false, description)
                            .addArgument(storm::settings::ArgumentBuilder::createStringArgument(fieldName, fieldDescription).build())
                            .build());
    };

    addStringOption(stringDiagramOption, "model check the given string diagram", "filename", "The name of the file to model check (json).");
    addStringOption(entranceName, "entrance to consider as the initial state of the string diagram", "entrance",
                    "<l|r><number> e.g. l5 is left entrance 5, default: l0");
    addStringOption(exitName, "exit to consider as the target state of the string diagram", "exit", "<l|r><number> e.g. r3 is right exit 3, default: r0");
    addStringOption(approachName, "approach to use for computing reachability in the string diagram", "approach",
                    "(choose from {monolithic, naive, weight}, default: monolithic");
    addStringOption(exportStringDiagramName, "export the string diagram to a dot file", "filename", "The name of the file to write the dot file");
    addStringOption(benchmarkDataName, "write benchmark results", "filename", "The path to store the benchmark results");
    addStringOption(paretoPrecisionTypeName, "multi objective computation precision type", "type", "In: {absolute, relative}");

    this->addOption(storm::settings::OptionBuilder(moduleName, paretoPrecisionName, false,
                                                   "the precision with which to perform multiobjective optimisation, see also --paretoPrecisionType")
                        .addArgument(storm::settings::ArgumentBuilder::createDoubleArgument("precision", "the relative or absolution precision").build())
                        .build());

    this->addOption(storm::settings::OptionBuilder(moduleName, oviEpsilonName, false,
                                                   "epsilon with which to perform optimistic (compositional) value iteration")
                        .addArgument(storm::settings::ArgumentBuilder::createDoubleArgument("epsilon", "").build())
                        .build());

    this->addOption(storm::settings::OptionBuilder(moduleName, paretoStepsName, false, "maximum number of steps to perform in the multiobjective optimisation")
                        .addArgument(storm::settings::ArgumentBuilder::createUnsignedIntegerArgument("steps", "number of steps").build())
                        .build());
}

bool ComposeIOSettings::check() const {
    return true;
}

void ComposeIOSettings::finalize() {}

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

bool ComposeIOSettings::isBenchmarkDataSet() const {
    return this->getOption(benchmarkDataName).getHasOptionBeenSet();
}

bool ComposeIOSettings::isParetoPrecisionSet() const {
    return this->getOption(paretoPrecisionName).getHasOptionBeenSet();
}

bool ComposeIOSettings::isParetoPrecisionTypeSet() const {
    return this->getOption(paretoPrecisionTypeName).getHasOptionBeenSet();
}

bool ComposeIOSettings::isParetoStepsSet() const {
    return this->getOption(paretoStepsName).getHasOptionBeenSet();
}

bool ComposeIOSettings::isOVIEpsilonSet() const {
    return this->getOption(oviEpsilonName).getHasOptionBeenSet();
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

std::string ComposeIOSettings::getBenchmarkDataFilename() const {
    return this->getOption(benchmarkDataName).getArgumentByName("filename").getValueAsString();
}

double ComposeIOSettings::getParetoPrecision() const {
    return this->getOption(paretoPrecisionName).getArgumentByName("precision").getValueAsDouble();
}

double ComposeIOSettings::getOVIEpsilon() const {
    if (isOVIEpsilonSet()) {
        return this->getOption(oviEpsilonName).getArgumentByName("epsilon").getValueAsDouble();
    } else {
        return 1e-4;
    }
}

std::string ComposeIOSettings::getParetoPrecisionType() const {
    return this->getOption(paretoPrecisionTypeName).getArgumentByName("type").getValueAsString();
}

size_t ComposeIOSettings::getParetoSteps() const {
    return this->getOption(paretoStepsName).getArgumentByName("steps").getValueAsUnsignedInteger();
}

}  // namespace modules
}  // namespace settings
}  // namespace storm
