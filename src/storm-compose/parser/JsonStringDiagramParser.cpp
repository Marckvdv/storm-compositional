#include "JsonStringDiagramParser.h"

#include "storm/adapters/RationalNumberAdapter.h"
#include "storm/exceptions/UnexpectedException.h"
#include "storm/io/file.h"

#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/models/PrismModel.h"
#include "storm-compose/models/Reference.h"
#include "storm-compose/models/SequenceModel.h"
#include "storm-compose/models/SumModel.h"
#include "storm-compose/models/TraceModel.h"

#include <sstream>

namespace storm {
namespace parser {

template<typename ValueType>
const std::string JsonStringDiagramParser<ValueType>::LEFT_ENTRANCE = ">|";
template<typename ValueType>
const std::string JsonStringDiagramParser<ValueType>::RIGHT_ENTRANCE = "|<";
template<typename ValueType>
const std::string JsonStringDiagramParser<ValueType>::LEFT_EXIT = "<|";
template<typename ValueType>
const std::string JsonStringDiagramParser<ValueType>::RIGHT_EXIT = "|>";

template<typename ValueType>
JsonStringDiagramParser<ValueType>::JsonStringDiagramParser(storm::json<ValueType> data, std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager,
                                                            boost::filesystem::path root)
    : data(data), manager(manager), root(root) {}

template<typename ValueType>
storm::json<ValueType> JsonStringDiagramParser<ValueType>::parseJson(const std::string& str) {
    // TODO update JSON library to support comments
    return storm::json<ValueType>::parse(str);
}

template<typename ValueType>
JsonStringDiagramParser<ValueType> JsonStringDiagramParser<ValueType>::fromFilePath(const std::string& path,
                                                                                    std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager) {
    std::ifstream f(path);
    std::stringstream buffer;
    buffer << f.rdbuf();
    storm::json<ValueType> value = JsonStringDiagramParser<ValueType>::parseJson(buffer.str());

    boost::filesystem::path root(path);
    root.remove_filename();

    return JsonStringDiagramParser<ValueType>(value, manager, root);
}

template<typename ValueType>
void JsonStringDiagramParser<ValueType>::parse() {
    storm::json<ValueType> rootData = data["root"];
    manager->setRoot(parseOpenMdp(rootData));

    storm::json<ValueType> components = data["components"];
    for (auto it = components.begin(); it != components.end(); ++it) {
        std::string name = it.key();
        const storm::json<ValueType> value = it.value();
        auto openMdp = parseOpenMdp(value);
        manager->addReference(name, openMdp);
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
    else if (type == "sum")
        return parseSumModel(data);
    else if (type == "sequence")
        return parseSequenceModel(data);
    else if (type == "trace")
        return parseTraceModel(data);
    else
        STORM_LOG_THROW(false, storm::exceptions::UnexpectedException, "type not supported (yet?)");
}

template<typename ValueType>
std::shared_ptr<storm::models::OpenMdp<ValueType>> JsonStringDiagramParser<ValueType>::parseReference(const storm::json<ValueType>& data) {
    std::string name = data.template get<std::string>();
    return std::make_shared<storm::models::Reference<ValueType>>(manager, name);
}

template<typename ValueType>
std::shared_ptr<storm::models::OpenMdp<ValueType>> JsonStringDiagramParser<ValueType>::parsePrismModel(const storm::json<ValueType>& data) {
    std::vector<std::string> lEntrance, rEntrance, lExit, rExit;
    if (data.count(LEFT_ENTRANCE) != 0)
        lEntrance = parseStateValuations(data[LEFT_ENTRANCE]);
    if (data.count(RIGHT_ENTRANCE) != 0)
        rEntrance = parseStateValuations(data[RIGHT_ENTRANCE]);
    if (data.count(LEFT_EXIT) != 0)
        lExit = parseStateValuations(data[LEFT_EXIT]);
    if (data.count(RIGHT_EXIT) != 0)
        rExit = parseStateValuations(data[RIGHT_EXIT]);

    boost::filesystem::path p(data["path"].template get<std::string>());
    std::string fullPath = (root / p).native();

    return std::static_pointer_cast<storm::models::OpenMdp<ValueType>>(
        std::make_shared<storm::models::PrismModel<ValueType>>(manager, fullPath, lEntrance, rEntrance, lExit, rExit));
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

template<typename ValueType>
std::shared_ptr<storm::models::OpenMdp<ValueType>> JsonStringDiagramParser<ValueType>::parseTraceModel(const storm::json<ValueType>& data) {
    std::shared_ptr<storm::models::OpenMdp<ValueType>> value = parseOpenMdp(data["value"]);
    int64_t left = data.count("left") == 0 ? 0 : data["left"].template get<int64_t>();
    int64_t right = data.count("right") == 0 ? 0 : data["right"].template get<int64_t>();
    STORM_LOG_ASSERT(left >= 0 && right >= 0, "left and right need to be positive");

    return std::make_shared<storm::models::TraceModel<ValueType>>(manager, value, left, right);
}

template<typename ValueType>
std::string JsonStringDiagramParser<ValueType>::parseStateValuation(const storm::json<ValueType>& data) {
    std::string str = data.template get<std::string>();
    return str;
}

template<typename ValueType>
std::vector<std::string> JsonStringDiagramParser<ValueType>::parseStateValuations(const storm::json<ValueType>& data) {
    std::vector<std::string> stateValuations;
    for (const auto& entry : data) {
        stateValuations.push_back(parseStateValuation(entry));
    }
    return stateValuations;
}

template class JsonStringDiagramParser<storm::RationalNumber>;
template class JsonStringDiagramParser<double>;

}  // namespace parser
}  // namespace storm
