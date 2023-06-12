#pragma once

#include "storm/adapters/JsonAdapter.h"
#include "storm-compose/models/OpenMdpManager.h"
#include "storm-compose/models/OpenMdp.h"

#include <string>
#include <memory>

namespace storm {
namespace parser {

template<typename ValueType>
class JsonStringDiagramParser {
    public:
    JsonStringDiagramParser(storm::json<ValueType> data, storm::models::OpenMdpManager<ValueType>& manager);
    static JsonStringDiagramParser<ValueType> fromString(const std::string& str, storm::models::OpenMdpManager<ValueType>& manager);
    static JsonStringDiagramParser<ValueType> fromFilePath(const std::string& path,  storm::models::OpenMdpManager<ValueType>& manager);

    /// Parse input (in-place, storing results in manager)
    void parse();

    std::shared_ptr<storm::models::OpenMdp<ValueType>> parseOpenMdp(const storm::json<ValueType>& data);
    std::shared_ptr<storm::models::OpenMdp<ValueType>> parseReference(const storm::json<ValueType>& data);
    std::shared_ptr<storm::models::OpenMdp<ValueType>> parsePrismModel(const storm::json<ValueType>& data);
    std::shared_ptr<storm::models::OpenMdp<ValueType>> parseSumModel(const storm::json<ValueType>& data);
    std::shared_ptr<storm::models::OpenMdp<ValueType>> parseSequenceModel(const storm::json<ValueType>& data);

    private:
    storm::json<ValueType> data;
    storm::models::OpenMdpManager<ValueType>& manager;
};

}  // namespace exceptions
}  // namespace parser
