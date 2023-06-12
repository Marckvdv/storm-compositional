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
    std::string getStringDiagramFilename() const;

    // The name of the module.
    static const std::string moduleName;
    static const std::string stringDiagramOption;
};

}  // namespace modules
}  // namespace settings
}  // namespace storm
