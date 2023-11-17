#pragma once

#include "storm-compose/benchmark/BenchmarkStats.h"
#include "storm-compose/models/OpenMdpManager.h"

#include "ApproximateReachabilityResult.h"

namespace storm {
namespace modelchecker {

class OpenMdpReachabilityTask {
    typedef std::pair<bool, size_t> EntranceExit;

   public:
    OpenMdpReachabilityTask() = default;
    OpenMdpReachabilityTask(EntranceExit entrance, EntranceExit exit) : entrance(entrance), exit(exit) {}

    std::string getEntranceLabel() {
        return (entrance.first ? "len" : "ren") + std::to_string(entrance.second);
    }

    std::string getExitLabel() {
        return (exit.first ? "lex" : "rex") + std::to_string(exit.second);
    }

    bool isLeftEntrance() {
        return entrance.first;
    }

    bool isLeftExit() {
        return exit.first;
    }

    size_t getEntranceId() {
        return entrance.second;
    }

    size_t getExitId() {
        return exit.second;
    }

   private:
    EntranceExit entrance{false, 0}, exit{true, 0};
};

// Interface for model checking string diagrams
template<typename ValueType>
class AbstractOpenMdpChecker {
   public:
    AbstractOpenMdpChecker(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager, storm::compose::benchmark::BenchmarkStats<ValueType>& stats)
        : manager(manager), stats(stats) {}

    virtual ApproximateReachabilityResult<ValueType> check(OpenMdpReachabilityTask task) = 0;

   protected:
    std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager;
    storm::compose::benchmark::BenchmarkStats<ValueType>& stats;
};

}  // namespace modelchecker
}  // namespace storm