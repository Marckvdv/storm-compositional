#pragma once

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

    std::string getStringDiagramFilename() const;
    std::string getEntrance() const;
    std::string getExit() const;
    std::string getApproach() const;
    std::string getExportStringDiagramFilename() const;
    std::string getBenchmarkDataFilename() const;
    double getParetoPrecision() const;
    std::string getParetoPrecisionType() const;
    size_t getParetoSteps() const;

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
};

}  // namespace modules
}  // namespace settings
}  // namespace storm
