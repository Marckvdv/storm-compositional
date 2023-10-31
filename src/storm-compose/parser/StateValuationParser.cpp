#include "StateValuationParser.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <ostream>

#include "storm/adapters/RationalNumberAdapter.h"
#include "storm/exceptions/InvalidOperationException.h"

namespace storm {
namespace parser {

StateValuationParser::StateValuationParser(const storm::storage::sparse::StateValuations& stateValuations) : stateValuations(stateValuations) {
    STORM_LOG_THROW(stateValuations.getNumberOfStates() > 0, storm::exceptions::InvalidOperationException, "Must have at least one state");

    integerOrder = {};
    for (auto it = stateValuations.at(0).begin(); it != stateValuations.at(0).end(); ++it) {
        if (it.isInteger()) {
            integerOrder.push_back(it.getName());
        }
    }
}

storm::storage::sparse::StateValuations::StateValuation StateValuationParser::parseStateValuation(std::string toParse) {
    std::vector<std::string> parts;
    std::vector<int64_t> integerValues;
    boost::algorithm::split(parts, toParse, boost::is_any_of("&"));
    for (auto& part : parts) {
        auto trimmed = boost::algorithm::trim_copy(part);

        std::vector<std::string> parts2;
        boost::algorithm::split(parts2, trimmed, boost::is_any_of("="));

        STORM_LOG_THROW(parts2.size() == 2, storm::exceptions::InvalidOperationException, "incorrect equality");
        int64_t number = std::atoi(parts2.at(1).c_str());
        integerValues.push_back(number);
    }

    return storm::storage::sparse::StateValuations::StateValuation({}, std::move(integerValues), {});
}

}
}
