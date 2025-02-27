#ifndef STRINGDIAGRAMMODELCHECKING_H_
#define STRINGDIAGRAMMODELCHECKING_H_

#include "storm/adapters/RationalNumberForward.h"
#include "storm/exceptions/InvalidArgumentException.h"
#include "storm-cli-utilities/cli.h"
#include "storm-cli-utilities/model-handling.h"
#include "storm/adapters/RationalNumberAdapter.h"
#include "storm/analysis/GraphConditions.h"
#include "storm/api/storm.h"
#include "storm/exceptions/NotSupportedException.h"
#include "storm/exceptions/UnexpectedException.h"
#include "storm/modelchecker/results/ExplicitQualitativeCheckResult.h"
#include "storm/settings/modules/DebugSettings.h"
#include "storm/settings/modules/GeneralSettings.h"
#include "storm/utility/NumberTraits.h"

#include "storm-compose-cli/settings/ComposeSettings.h"
#include "storm-compose-cli/settings/modules/ComposeIOSettings.h"
#include "storm-compose/models/visitor/BenchmarkStatsVisitor.h"
#include "storm-compose/models/visitor/FlatMdpBuilderVisitor.h"
#include "storm-compose/models/visitor/ParetoVisitor.h"
#include "storm-compose/parser/JsonStringDiagramParser.h"

#include "storm-compose/benchmark/BenchmarkStats.h"
#include "storm-compose/modelchecker/AbstractOpenMdpChecker.h"
#include "storm-compose/modelchecker/CompositionalValueIteration.h"
#include "storm-compose/modelchecker/MonolithicOpenMdpChecker.h"
#include "storm-compose/modelchecker/NaiveOpenMdpChecker.h"

#include "storm-parsers/parser/ExpressionParser.h"

namespace storm {
namespace compose {
namespace cli {

enum ReachabilityCheckingApproach {
    MONOLITHIC,
    NAIVE,
    COMPOSITIONAL_VI,
};

template<typename ValueType>
struct ReachabilityCheckingOptions {
    ReachabilityCheckingOptions() = default;

    std::shared_ptr<storm::models::OpenMdpManager<ValueType>> omdpManager;
    std::pair<bool, size_t> entrance{false, 0}, exit{true, 0};
    ReachabilityCheckingApproach approach = MONOLITHIC;
    boost::optional<std::string> benchmarkStatsPath;
};

boost::optional<std::pair<bool, size_t>> parseEntranceExit(std::string text);

template<typename ValueType>
boost::optional<ReachabilityCheckingOptions<ValueType>> processStringDiagramOptions();

template<typename ValueType>
void performStringDiagramModelChecking(ReachabilityCheckingOptions<ValueType>& options);

}  // namespace cli
}  // namespace compose
}  // namespace storm

#endif  // STRINGDIAGRAMMODELCHECKING_H_
