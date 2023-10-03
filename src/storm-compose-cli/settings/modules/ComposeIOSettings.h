#pragma once

#include "storm-config.h"
#include "storm/settings/modules/ModuleSettings.h"

namespace storm {
namespace settings {
namespace modules {

/*!
 * This class represents the settings for POMDP model checking.
 */
class ComposeIOSettings : public ModuleSettings {
   public:
    /*!
     * Creates a new set of POMDP settings.
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

    std::string getStringDiagramFilename() const;
    std::string getEntrance() const;
    std::string getExit() const;
    std::string getApproach() const;
    std::string getExportStringDiagramFilename() const;
    std::string getBenchmarkDataFilename() const;

    // The name of the module.
    static const std::string moduleName;
    static const std::string stringDiagramOption;
    static const std::string entranceName;
    static const std::string exitName;
    static const std::string approachName;
    static const std::string exportStringDiagramName;
    static const std::string benchmarkDataName;
};

}  // namespace modules
}  // namespace settings
}  // namespace storm
