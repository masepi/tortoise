#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

namespace tortoise {

class Position;


struct PerftResult {
	uint64_t count = 0;

	struct MoveWithCount {
		std::string move;
		uint64_t count;

		MoveWithCount(std::string_view _move, uint64_t _count): move(_move), count(_count) {}
	};

	std::vector<MoveWithCount> moves;

	void sort_by_move() {
		std::sort(moves.begin(), moves.end(), [](const MoveWithCount& first, const MoveWithCount& second) { return first.move < second.move; });
	}
};

PerftResult perft(const Position& p, int depth, size_t thread_count = 0);

} // namespace tortoise 


