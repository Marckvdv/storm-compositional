#pragma once

#include "storm/adapters/JsonAdapter.h"
#include "storm/utility/Stopwatch.h"

namespace storm {
namespace compose {
namespace benchmark {

template<class ValueType>
struct BenchmarkStats {
    BenchmarkStats() = default;

    storm::utility::Stopwatch totalTime, modelBuildingTime, reachabilityComputationTime;
    size_t stateCount = 0, stringDiagramDepth = 0, uniqueLeaves = 0, leafStates = 0;
    size_t sequenceCount = 0, sumCount = 0, traceCount = 0;
    size_t paretoPoints = 0;
    ValueType lowerBound = storm::utility::zero<ValueType>(), upperBound = storm::utility::one<ValueType>();
    storm::RationalNumber leafSchedulerCount = storm::utility::zero<storm::RationalNumber>();

    constexpr static double NANOSECONDS_TO_SECONDS = 1e-9;

    storm::json<ValueType> toJson() {
        storm::json<ValueType> result;

        result["totalTime"] = totalTime.getTimeInNanoseconds() * NANOSECONDS_TO_SECONDS;
        result["modelBuildingTime"] = modelBuildingTime.getTimeInNanoseconds() * NANOSECONDS_TO_SECONDS;
        result["reachabilityComputationTime"] = reachabilityComputationTime.getTimeInNanoseconds() * NANOSECONDS_TO_SECONDS;

        result["stateCount"] = stateCount;
        result["stringDiagramDepth"] = stringDiagramDepth;
        result["uniqueLeaves"] = uniqueLeaves;
        result["leafStates"] = leafStates;

        result["sequenceCount"] = sequenceCount;
        result["sumCount"] = sumCount;
        result["traceCount"] = traceCount;

        result["lowerBound"] = lowerBound;
        result["upperBound"] = upperBound;
        ValueType gap = upperBound - lowerBound;
        result["gap"] = gap;

        result["paretoPoints"] = paretoPoints;

        storm::RationalNumber schedulerLimit = storm::utility::pow(storm::RationalNumber(2), 64);
        if (leafSchedulerCount > schedulerLimit) {
            result["leafSchedulerCount"] = ">2^64";
        } else {
            result["leafSchedulerCount"] = storm::utility::to_string(leafSchedulerCount);
        }

        return result;
    }
};

}  // namespace benchmark
}  // namespace compose
}  // namespace storm