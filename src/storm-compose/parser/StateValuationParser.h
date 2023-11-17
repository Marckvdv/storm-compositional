#pragma once

#include "storm/storage/sparse/StateValuations.h"

namespace storm {
namespace parser {

class StateValuationParser {
   public:
    StateValuationParser(const storm::storage::sparse::StateValuations& stateValuations);
    storm::storage::sparse::StateValuations::StateValuation parseStateValuation(std::string toParse);

   private:
    const storm::storage::sparse::StateValuations& stateValuations;

    // std::vector<std::string> booleanOrder;
    std::vector<std::string> integerOrder;
};

}  // namespace parser
}  // namespace storm
