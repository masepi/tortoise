
#include <string>
#include <memory>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <chrono>

#include "rules.h"
#include "perft.h"
#include "fen.h"
#include "epd.h"

#include "divide.h"

using namespace tortoise;

void debug_position(std::string_view fen, int depth)
{
	const std::optional<DivideResult> result = divide(fen, depth);
	if (!result) {
		std::cout << "No differences found.\n";
		return;
	}

	std::cout << "FEN: " << result->fen << "\n\n";
	std::cout << std::left
		<< std::setw(8) << "Move"
		<< std::setw(14) << "Stockfish"
		<< std::setw(14) << "Tortoise"
		<< "Result\n";

	const auto& stockfish_moves = result->stockfish_result.moves;
	const auto& tortoise_moves = result->tortoise_result.moves;
	size_t stockfish_index = 0;
	size_t tortoise_index = 0;
	while (stockfish_index < stockfish_moves.size() || tortoise_index < tortoise_moves.size()) {
		const bool has_stockfish_move = stockfish_index < stockfish_moves.size();
		const bool has_tortoise_move = tortoise_index < tortoise_moves.size();
		const bool take_stockfish = has_stockfish_move &&
			(!has_tortoise_move || stockfish_moves[stockfish_index].move < tortoise_moves[tortoise_index].move);
		const bool take_tortoise = has_tortoise_move &&
			(!has_stockfish_move || tortoise_moves[tortoise_index].move < stockfish_moves[stockfish_index].move);

		std::string move;
		std::string stockfish_count = "-";
		std::string tortoise_count = "-";
		if (take_stockfish) {
			move = stockfish_moves[stockfish_index].move;
			stockfish_count = std::to_string(stockfish_moves[stockfish_index].count);
			stockfish_index++;
		} else if (take_tortoise) {
			move = tortoise_moves[tortoise_index].move;
			tortoise_count = std::to_string(tortoise_moves[tortoise_index].count);
			tortoise_index++;
		} else {
			move = stockfish_moves[stockfish_index].move;
			stockfish_count = std::to_string(stockfish_moves[stockfish_index].count);
			tortoise_count = std::to_string(tortoise_moves[tortoise_index].count);
			stockfish_index++;
			tortoise_index++;
		}

		std::cout << std::setw(8) << move
			<< std::setw(14) << stockfish_count
			<< std::setw(14) << tortoise_count
			<< (stockfish_count == tortoise_count ? "OK" : "FAILED") << "\n";
	}

	std::cout << "\nTotal: "
		<< result->stockfish_result.count << " / "
		<< result->tortoise_result.count << "\n";
}

int main(int argc, char* argv[])
{
	tortoise::initialize();

	// debug_position(fen, depth);
	// return 0;

	if (argc != 2) {
		printf("Wrong number of parameters! Should be 2.");
		return -1;
	}

	std::vector<PerftTestCase> test_cases = parse_perft_test_folder(argv[1]);

	const char* RED = "\033[31m";
    const char* GREEN = "\033[32m";
    const char* RESET = "\033[0m";

	const auto start = std::chrono::steady_clock::now();
	
	int passed_count = 0;
	uint64_t total_nodes_count = 0;
	for (const auto& test_case: test_cases) {
		const Position position = Fen::load(test_case.fen);
		const uint64_t count = perft(position, test_case.depth).count;

		if (count == test_case.count) {
			passed_count++;
			std::cout << GREEN << "[PASSED] " << RESET;
		} else {
			std::cout << RED << "[FAILED] " << RESET;
		}
		std::cout << test_case.fen << " ;D" << test_case.depth << " " << test_case.count << "\n";
		total_nodes_count += count;
	}

	const auto finish = std::chrono::steady_clock::now();
	const auto duration = (finish-start);
	const auto mnps = total_nodes_count / std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

	std::cout << "\n" << "Passed = " << passed_count << " / " << test_cases.size() << "\n";
	std::cout << "Count = " << total_nodes_count << "\n";
	std::cout << "Speed = " << mnps << " Mps\n";

	return 0;
}
