#include "EntranceExit.h"
#include "storm/utility/macros.h"

namespace storm {
namespace storage {

EntranceExit match(EntranceExit entranceExit) {
    switch (entranceExit) {
        case L_ENTRANCE:
            return R_EXIT;

        case R_ENTRANCE:
            return L_EXIT;

        case L_EXIT:
            return R_ENTRANCE;

        case R_EXIT:
            return L_ENTRANCE;

        default:
            STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "sanity check failed");
            return L_ENTRANCE;
    }
}

std::string entranceExitToString(EntranceExit entranceExit) {
    switch (entranceExit) {
        case L_ENTRANCE:
            return "L_ENTRANCE";

        case R_ENTRANCE:
            return "R_ENTRANCE";

        case L_EXIT:
            return "L_EXIT";

        case R_EXIT:
            return "R_EXIT";

        default:
            STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "sanity check failed");
            return "";
    }
}

Position positionMatch(Position position) {
    return {match(position.first), position.second};
}

std::string positionToString(Position position) {
    return entranceExitToString(position.first) + " " + std::to_string(position.second);
}

}  // namespace storage
}  // namespace storm
