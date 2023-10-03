#pragma once

#include "storm/utility/Stopwatch.h"
#include "storm/adapters/JsonAdapter.h"

namespace storm {
namespace compose {
namespace benchmark {

template <class ValueType>
struct BenchmarkStats {
    BenchmarkStats() = default;

    storm::utility::Stopwatch totalTime;
    size_t stateCount = 0, stringDiagramDepth = 0, uniqueLeaves = 0, leafStates = 0;
    size_t sequenceCount = 0, sumCount = 0, traceCount = 0;
    ValueType lowerBound = storm::utility::zero<ValueType>(), upperBound = storm::utility::one<ValueType>();
    storm::RationalNumber leafSchedulerCount = storm::utility::zero<storm::RationalNumber>();

    constexpr static double NANOSECONDS_TO_SECONDS = 1e-9;

    storm::json<ValueType> toJson() {
        storm::json<ValueType> result;

        result["totalTime"] = totalTime.getTimeInNanoseconds() * NANOSECONDS_TO_SECONDS;
        result["stateCount"] = stateCount;
        result["stringDiagramDepth"] = stringDiagramDepth;
        result["uniqueLeaves"] = uniqueLeaves;
        result["leafStates"] = leafStates;

        result["sequenceCount"] = sequenceCount;
        result["sumCount"] = sumCount;
        result["traceCount"] = traceCount;

        result["lowerBound"] = lowerBound;
        result["upperBound"] = upperBound;

        storm::RationalNumber schedulerLimit = storm::utility::pow(storm::RationalNumber(2), 64);
        if (leafSchedulerCount > schedulerLimit) {
            result["leafSchedulerCount"] = ">2^64";
        } else {
            result["leafSchedulerCount"] = storm::utility::to_string(leafSchedulerCount);
        }

        return result;
    }
};

}
}
}