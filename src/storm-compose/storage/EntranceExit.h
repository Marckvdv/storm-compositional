#pragma once

#include "storm/exceptions/InvalidOperationException.h"

namespace storm {
namespace storage {

enum EntranceExit { L_ENTRANCE, R_ENTRANCE, L_EXIT, R_EXIT };
EntranceExit match(EntranceExit entranceExit);
std::string entranceExitToString(EntranceExit entranceExit);

typedef std::pair<EntranceExit, size_t> Position;
Position positionMatch(Position position);
std::string positionToString(Position position);

}  // namespace storage
}  // namespace storm
