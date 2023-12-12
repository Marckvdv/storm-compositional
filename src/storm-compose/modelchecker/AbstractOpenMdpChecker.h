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

    std::string getEntranceLabel() const {
        return (entrance.first ? "len" : "ren") + std::to_string(entrance.second);
    }

    std::string getExitLabel() const {
        return (exit.first ? "lex" : "rex") + std::to_string(exit.second);
    }

    bool isLeftEntrance() const {
        return entrance.first;
    }

    bool isLeftExit() const {
        return exit.first;
    }

    size_t getEntranceId() const {
        return entrance.second;
    }

    size_t getExitId() const {
        return exit.second;
    }

    template<typename ValueType>
    std::vector<ValueType> toExitWeights(size_t lExits, size_t rExits) const {
        std::vector<ValueType> weights(lExits + rExits, 0);
        size_t index = exit.second;
        if (!exit.first) {
            index += lExits;
        }
        weights[index] = 1;
        return weights;
    }

   private:
    EntranceExit entrance{true, 0}, exit{false, 0};
};

// Interface for model checking string diagrams
template<typename ValueType>
class AbstractOpenMdpChecker {
   public:
    AbstractOpenMdpChecker(std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager, storm::compose::benchmark::BenchmarkStats<ValueType>& stats)
        : manager(manager), stats(stats) {}
    virtual ~AbstractOpenMdpChecker() {}
    virtual ApproximateReachabilityResult<ValueType> check(OpenMdpReachabilityTask task) = 0;

   protected:
    std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager;
    storm::compose::benchmark::BenchmarkStats<ValueType>& stats;
};

}  // namespace modelchecker
}  // namespace storm
