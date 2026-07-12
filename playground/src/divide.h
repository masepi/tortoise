#pragma once

#include <string>
#include <utility>
#include <optional>

#include "perft.h"

namespace tortoise {

struct DivideResult {
	std::string fen;
	PerftResult stockfish_result;
	PerftResult tortoise_result;
};

std::optional<DivideResult> divide(std::string_view fen, int depth);


}