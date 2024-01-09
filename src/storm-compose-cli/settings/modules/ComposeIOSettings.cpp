#include "storm-compose-cli/settings/modules/ComposeIOSettings.h"

#include "storm-compose/modelchecker/CompositionalValueIteration.h"
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
const std::string ComposeIOSettings::cviStepsName = "cviSteps";
const std::string ComposeIOSettings::oviEpsilonName = "oviEpsilon";
const std::string ComposeIOSettings::cacheMethodName = "cacheMethod";
const std::string ComposeIOSettings::useOviName = "useOvi";
const std::string ComposeIOSettings::useBottomUpName = "useBottomUp";
const std::string ComposeIOSettings::paretoCacheEpsilonName = "paretoCacheEpsilon";
const std::string ComposeIOSettings::oviIntervalName = "oviInterval";
const std::string ComposeIOSettings::bottomUpIntervalName = "bottomUpInterval";

ComposeIOSettings::ComposeIOSettings() : ModuleSettings(moduleName) {
    addStringOption(stringDiagramOption, "load the given string diagram", "filename", "The path of the file to load (json).");
    addStringOption(entranceName, "entrance to consider as the initial state of the string diagram", "entrance",
                    "<l|r><number> e.g. l5 is left entrance 5, default: l0");
    addStringOption(exitName, "exit to consider as the target state of the string diagram", "exit", "<l|r><number> e.g. r3 is right exit 3, default: r0");
    addStringOption(approachName, "approach to use for computing reachability in the string diagram", "approach",
                    "(choose from {monolithic, naive2}, default: monolithic");
    addStringOption(exportStringDiagramName, "export the string diagram to a dot file", "filename", "The name of the file to write the dot file");
    addStringOption(benchmarkDataName, "write benchmark results", "filename", "The path to store the benchmark results");
    addStringOption(paretoPrecisionTypeName, "multi objective computation precision type", "type", "In: {absolute, relative}");
    addStringOption(cacheMethodName, "Cache method to use", "method", "In: {no, exact, pareto} (default=pareto)");

    addDoubleOption(paretoPrecisionName, "the precision with which to perform multiobjective optimisation, see also --paretoPrecisionType", "precision",
                    "the relative or absolution precision");
    addDoubleOption(oviEpsilonName, "epsilon with which to perform optimistic (compositional) value iteration", "epsilon", "");
    addDoubleOption(paretoCacheEpsilonName, "error tolerance for using the cache", "epsilon", "");

    addUnsignedOption(paretoStepsName, "maximum number of steps to perform in the multiobjective optimisation", "steps", "number of steps");
    addUnsignedOption(cviStepsName, "maximum number of steps to perform in CVI", "steps", "number of steps");

    addFlag(useOviName, "use OVI termination");
    addFlag(useBottomUpName, "use bottom-up termination");
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

bool ComposeIOSettings::isCviStepsSet() const {
    return this->getOption(cviStepsName).getHasOptionBeenSet();
}

bool ComposeIOSettings::isOviEpsilonSet() const {
    return this->getOption(oviEpsilonName).getHasOptionBeenSet();
}

bool ComposeIOSettings::isCacheMethodSet() const {
    return this->getOption(cacheMethodName).getHasOptionBeenSet();
}

bool ComposeIOSettings::isParetoCacheEpsilonSet() const {
    return this->getOption(paretoCacheEpsilonName).getHasOptionBeenSet();
}

bool ComposeIOSettings::useOvi() const {
    return this->getOption(useOviName).getHasOptionBeenSet();
}

bool ComposeIOSettings::useBottomUp() const {
    return this->getOption(useBottomUpName).getHasOptionBeenSet();
}

bool ComposeIOSettings::isOviIntervalSet() const {
    return this->getOption(oviIntervalName).getHasOptionBeenSet();
}

bool ComposeIOSettings::isBottomUpIntervalSet() const {
    return this->getOption(bottomUpIntervalName).getHasOptionBeenSet();
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

double ComposeIOSettings::getOviEpsilon() const {
    if (isOviEpsilonSet()) {
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

storm::modelchecker::CacheMethod ComposeIOSettings::getCacheMethod() const {
    if (!isCacheMethodSet()) {
        return modelchecker::PARETO_CACHE;
    }

    auto cacheMethodString = this->getOption(cacheMethodName).getArgumentByName("method").getValueAsString();
    if (cacheMethodString == "no") {
        return modelchecker::NO_CACHE;
    } else if (cacheMethodString == "exact") {
        return modelchecker::EXACT_CACHE;
    } else if (cacheMethodString == "pareto") {
        return modelchecker::PARETO_CACHE;
    } else {
        STORM_LOG_THROW(false, storm::exceptions::InvalidArgumentException, "Unknown cache method: " << cacheMethodString);
    }
}

double ComposeIOSettings::getParetoCacheEpsilon() const {
    if (isParetoCacheEpsilonSet()) {
        return this->getOption(paretoCacheEpsilonName).getArgumentByName("epsilon").getValueAsDouble();
    } else {
        return 1e-2;
    }
}
size_t ComposeIOSettings::getCviSteps() const {
    if (isCviStepsSet()) {
        return this->getOption(cviStepsName).getArgumentByName("steps").getValueAsUnsignedInteger();
    } else {
        return 200;
    }
}

size_t ComposeIOSettings::getOviInterval() const {
    if (isOviIntervalSet()) {
        return this->getOption(oviIntervalName).getArgumentByName("steps").getValueAsUnsignedInteger();
    } else {
        return 10;
    }
}

size_t ComposeIOSettings::getBottomUpInterval() const {
    if (isBottomUpIntervalSet()) {
        return this->getOption(bottomUpIntervalName).getArgumentByName("steps").getValueAsUnsignedInteger();
    } else {
        return 10;
    }
}

void ComposeIOSettings::addStringOption(std::string optionName, std::string description, std::string fieldName, std::string fieldDescription) {
    this->addOption(storm::settings::OptionBuilder(moduleName, optionName, false, description)
                        .addArgument(storm::settings::ArgumentBuilder::createStringArgument(fieldName, fieldDescription).build())
                        .build());
}

void ComposeIOSettings::addDoubleOption(std::string optionName, std::string description, std::string fieldName, std::string fieldDescription) {
    this->addOption(storm::settings::OptionBuilder(moduleName, optionName, false, description)
                        .addArgument(storm::settings::ArgumentBuilder::createDoubleArgument(fieldName, fieldDescription).build())
                        .build());
}

void ComposeIOSettings::addUnsignedOption(std::string optionName, std::string description, std::string fieldName, std::string fieldDescription) {
    this->addOption(storm::settings::OptionBuilder(moduleName, optionName, false, description)
                        .addArgument(storm::settings::ArgumentBuilder::createUnsignedIntegerArgument(fieldName, fieldDescription).build())
                        .build());
}

void ComposeIOSettings::addFlag(std::string optionName, std::string description) {
    this->addOption(storm::settings::OptionBuilder(moduleName, optionName, false, description).build());
}

}  // namespace modules
}  // namespace settings
}  // namespace storm
