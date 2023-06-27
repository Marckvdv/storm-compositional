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
    typedef std::string StateValuation;

    JsonStringDiagramParser(storm::json<ValueType> data, storm::models::OpenMdpManager<ValueType>& manager);
    static JsonStringDiagramParser<ValueType> fromString(const std::string& str, storm::models::OpenMdpManager<ValueType>& manager);
    static JsonStringDiagramParser<ValueType> fromFilePath(const std::string& path,  storm::models::OpenMdpManager<ValueType>& manager);

    const static std::string LEFT_ENTRANCE, RIGHT_ENTRANCE, LEFT_EXIT, RIGHT_EXIT;

    /// Parse input (in-place, storing results in manager)
    void parse();

    private:
    std::shared_ptr<storm::models::OpenMdp<ValueType>> parseOpenMdp(const storm::json<ValueType>& data);
    std::shared_ptr<storm::models::OpenMdp<ValueType>> parseReference(const storm::json<ValueType>& data);
    std::shared_ptr<storm::models::OpenMdp<ValueType>> parsePrismModel(const storm::json<ValueType>& data);
    std::shared_ptr<storm::models::OpenMdp<ValueType>> parseSumModel(const storm::json<ValueType>& data);
    std::shared_ptr<storm::models::OpenMdp<ValueType>> parseSequenceModel(const storm::json<ValueType>& data);
    std::shared_ptr<storm::models::OpenMdp<ValueType>> parseTraceModel(const storm::json<ValueType>& data);

    StateValuation parseStateValuation(const storm::json<ValueType>& data);
    std::vector<StateValuation> parseStateValuations(const storm::json<ValueType>& data);

    storm::json<ValueType> data;
    storm::models::OpenMdpManager<ValueType>& manager;
};

}  // namespace exceptions
}  // namespace parser
