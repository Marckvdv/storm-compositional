#include "JsonStringDiagramParser.h"

#include "storm/io/file.h"
#include "storm/exceptions/UnexpectedException.h"
#include "storm/adapters/RationalNumberAdapter.h"

#include "storm-compose/models/Reference.h"
#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/models/PrismModel.h"
#include "storm-compose/models/SumModel.h"
#include "storm-compose/models/SequenceModel.h"

#include <sstream>

namespace storm {
namespace parser {

template<typename ValueType>
JsonStringDiagramParser<ValueType>::JsonStringDiagramParser(storm::json<ValueType> data, storm::models::OpenMdpManager<ValueType>& manager)
    : data(data), manager(manager) {
}

template<typename ValueType>
JsonStringDiagramParser<ValueType> JsonStringDiagramParser<ValueType>::fromString(const std::string& str, storm::models::OpenMdpManager<ValueType>& manager) {
    return JsonStringDiagramParser<ValueType>(storm::json<ValueType>::parse(str), manager);
}

template<typename ValueType>
JsonStringDiagramParser<ValueType> JsonStringDiagramParser<ValueType>::fromFilePath(const std::string& path, storm::models::OpenMdpManager<ValueType>& manager) {
    std::ifstream f(path);
    storm::json<ValueType> value = storm::json<ValueType>::parse(f);
    return JsonStringDiagramParser<ValueType>(value, manager);
}

template<typename ValueType>
void JsonStringDiagramParser<ValueType>::parse() {
    storm::json<ValueType> rootData = data["root"];
    manager.setRoot(parseOpenMdp(rootData));

    storm::json<ValueType> components = data["components"];
    for (auto it = components.begin(); it != components.end(); ++it) {
        std::string name = it.key();
        const storm::json<ValueType> value = it.value();
        auto openMdp = parseOpenMdp(value);
        manager.addReference(name, openMdp);
    }
}

template<typename ValueType>
std::shared_ptr<storm::models::OpenMdp<ValueType>> JsonStringDiagramParser<ValueType>::parseOpenMdp(const storm::json<ValueType>& data) {
    if (data.is_string()) {
        auto ref = parseReference(data);
        return ref;
    }

    STORM_LOG_THROW(data.is_object(), storm::exceptions::UnexpectedException, "expected object");

    std::string type = data["type"].template get<std::string>();
    if (type == "prism")
        return parsePrismModel(data);
    if (type == "sum")
        return parseSumModel(data);
    if (type == "sequence")
        return parseSequenceModel(data);
}

template<typename ValueType>
std::shared_ptr<storm::models::OpenMdp<ValueType>> JsonStringDiagramParser<ValueType>::parseReference(const storm::json<ValueType>& data) {
    std::string name = data.template get<std::string>();
    return std::make_shared<storm::models::Reference<ValueType>>(manager, name);
}

template<typename ValueType>
std::shared_ptr<storm::models::OpenMdp<ValueType>> JsonStringDiagramParser<ValueType>::parsePrismModel(const storm::json<ValueType>& data) {
    std::string path = data["path"].template get<std::string>();
    return std::make_shared<storm::models::PrismModel<ValueType>>(manager, path);
}

template<typename ValueType>
std::shared_ptr<storm::models::OpenMdp<ValueType>> JsonStringDiagramParser<ValueType>::parseSumModel(const storm::json<ValueType>& data) {
    std::vector<std::shared_ptr<storm::models::OpenMdp<ValueType>>> values;
    for (auto& element : data["values"]) {
        values.push_back(parseOpenMdp(element));
    }

    return std::make_shared<storm::models::SumModel<ValueType>>(manager, values);
}

template<typename ValueType>
std::shared_ptr<storm::models::OpenMdp<ValueType>> JsonStringDiagramParser<ValueType>::parseSequenceModel(const storm::json<ValueType>& data) {
    std::vector<std::shared_ptr<storm::models::OpenMdp<ValueType>>> values;
    for (auto& element : data["values"]) {
        values.push_back(parseOpenMdp(element));
    }

    return std::make_shared<storm::models::SequenceModel<ValueType>>(manager, values);
}

template class JsonStringDiagramParser<storm::RationalNumber>;
template class JsonStringDiagramParser<double>;

}  // namespace exceptions
}  // namespace parser
