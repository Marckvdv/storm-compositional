#pragma once

#include "storm-compose/modelchecker/CompositionalValueIteration.h"
#include "storm-config.h"
#include "storm/settings/modules/ModuleSettings.h"

namespace storm {
namespace settings {
namespace modules {

/*!
 * This class represents the settings for compositional model checking.
 */
class ComposeIOSettings : public ModuleSettings {
   public:
    /*!
     * Creates a new set of compositional settings.
     */
    ComposeIOSettings();

    virtual ~ComposeIOSettings() = default;

    bool check() const override;
    void finalize() override;

    bool isStringDiagramSet() const;
    bool isEntranceSet() const;
    bool isExitSet() const;
    bool isApproachSet() const;
    bool isExportStringDiagramSet() const;
    bool isBenchmarkDataSet() const;
    bool isParetoPrecisionSet() const;
    bool isParetoPrecisionTypeSet() const;
    bool isParetoStepsSet() const;
    bool isCviStepsSet() const;
    bool isOviEpsilonSet() const;
    bool isCacheMethodSet() const;
    bool isParetoCacheEpsilonSet() const;
    bool useOvi() const;
    bool useBottomUp() const;
    bool isOviIntervalSet() const;
    bool isBottomUpIntervalSet() const;
    bool isIterationOrderSet() const;
    bool isLocalOviEpsilonSet() const;
    bool isUseRecursiveParetoComputationSet() const;

    std::string getStringDiagramFilename() const;
    std::string getEntrance() const;
    std::string getExit() const;
    std::string getApproach() const;
    std::string getExportStringDiagramFilename() const;
    std::string getBenchmarkDataFilename() const;
    std::string getParetoPrecisionType() const;
    std::string getIterationOrder() const;
    double getParetoPrecision() const;
    size_t getParetoSteps() const;
    size_t getCviSteps() const;
    double getOviEpsilon() const;
    double getLocalOviEpsilon() const;
    storm::modelchecker::CacheMethod getCacheMethod() const;
    double getParetoCacheEpsilon() const;
    size_t getOviInterval() const;
    size_t getBottomUpInterval() const;

    // The name of the module.
    static const std::string moduleName;
    static const std::string stringDiagramOption;
    static const std::string entranceName;
    static const std::string exitName;
    static const std::string approachName;
    static const std::string exportStringDiagramName;
    static const std::string benchmarkDataName;
    static const std::string paretoPrecisionName;
    static const std::string paretoPrecisionTypeName;
    static const std::string paretoStepsName;
    static const std::string cviStepsName;
    static const std::string oviEpsilonName;
    static const std::string cacheMethodName;
    static const std::string useOviName;
    static const std::string useBottomUpName;
    static const std::string paretoCacheEpsilonName;
    static const std::string oviIntervalName;
    static const std::string bottomUpIntervalName;
    static const std::string iterationOrderName;
    static const std::string localOviEpsilonName;
    static const std::string useRecursiveParetoComputationName;

   private:
    void addStringOption(std::string optionName, std::string description, std::string fieldName, std::string fieldDescription);
    void addDoubleOption(std::string optionName, std::string description, std::string fieldName, std::string fieldDescription);
    void addUnsignedOption(std::string optionName, std::string description, std::string fieldName, std::string fieldDescription);
    void addFlag(std::string optionName, std::string description);
};

}  // namespace modules
}  // namespace settings
}  // namespace storm
