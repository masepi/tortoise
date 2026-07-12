#include "perft.h"

#include <thread>
#include <future>
#include <memory>
#include <type_traits>
#include <cassert>
#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <unordered_map>

#include "debug.h"
#include "rules.h"
#include "zobrist.h"
#include "fen.h"

#include "thread_pool_executor.h"

namespace tortoise {

typedef std::unordered_map<Zobrist::Hash, std::pair<int, uint64_t>> TranspositionTablePerft;


static inline uint64_t do_perft(const Position& position, int depth)
{
	MoveList moves;
	position.analyze_moves(moves);

	if (depth == 1) {
		return moves.size();
	}

	uint64_t total = 0;
	for (Move move : moves) {
		Position position_after_move = position;
		position_after_move.make_move<false>(move);

		const uint64_t count = do_perft(position_after_move, depth-1);
		total += count;
	}
	return total;
}

PerftResult perft(const Position& position, int depth, size_t thread_count)
{
	PerftResult result;

	MoveList moves;
	position.analyze_moves(moves);
	result.moves.reserve(moves.size());

	if (depth == 1) {
		result.count = moves.size();
		for (Move move: moves) {
			result.moves.emplace_back(move_to_long_notation(move), 1ull);
		}
		return result;
	}

	if (thread_count == 0) {
		thread_count = std::thread::hardware_concurrency();
		if (thread_count == 0) {
			thread_count = 1;
		}
	}

	ThreadPoolExecutor executor{thread_count};

	std::vector<std::future<uint64_t>> futures;
	futures.reserve(moves.size());
	for (Move move : moves) {
		Position position_after_move = position;
		position_after_move.make_move<false>(move);

		futures.push_back(executor.submit(do_perft, position_after_move, depth-1));
	}
	assert(futures.size() == moves.size());

	for (size_t i = 0; i < futures.size(); i++) {
		auto& f = futures[i];
		const Move move = moves[i];
		f.wait();

		const uint64_t count = f.get();

		result.count += count;
		result.moves.emplace_back(move_to_long_notation(move), count);
	}
	return result;
}

} // namespace tortoise 
