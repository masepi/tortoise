#include "divide.h"

#include <array>
#include <charconv>
#include <cstdio>
#include <format>
#include <memory>
#include <string>

#include "fen.h"
#include "rules.h"

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

namespace tortoise {

static std::string exec(const char* command)
{
	std::array<char, 128> buffer;
	std::string result;

	std::unique_ptr<FILE, void(*)(FILE*)> pipe(POPEN(command, "r"), [](FILE* file) { PCLOSE(file); });
	if (!pipe) {
		return result;
	}

	while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}

static std::string stockfish_command(std::string_view fen, int depth)
{
#ifdef _WIN32
	return std::format("(echo position fen {} & echo go perft {} & echo quit) | stockfish", fen, depth);
#else
	return std::format("printf 'position fen {}\\ngo perft {}\\nquit\\n' | stockfish", fen, depth);
#endif
}

static PerftResult calc_stockfish_moves(std::string_view fen, int depth)
{
	const std::string command = stockfish_command(fen, depth);
	const std::string output = exec(command.c_str());

	PerftResult result;
	size_t line_begin = 0;
	while (line_begin < output.size()) {
		const size_t line_end = output.find('\n', line_begin);
		const size_t colon = output.find(':', line_begin);
		const size_t end = line_end == std::string::npos ? output.size() : line_end;

		if (colon < end) {
			const std::string_view move{output.data() + line_begin, colon - line_begin};
			const char* count_begin = output.data() + colon + 1;
			const char* count_end = output.data() + end;
			while (count_begin < count_end && *count_begin == ' ') {
				count_begin++;
			}

			uint64_t count = 0;
			const auto parse_result = std::from_chars(count_begin, count_end, count);
			if ((move.size() == 4 || move.size() == 5) && parse_result.ec == std::errc{}) {
				result.count += count;
				result.moves.emplace_back(move, count);
			}
		}

		if (line_end == std::string::npos) {
			break;
		}
		line_begin = line_end + 1;
	}
	return result;
}

static PerftResult calc_tortoise_moves(std::string_view fen, int depth)
{
	const Position position = Fen::load(fen);
	return perft(position, depth);
}

static DivideResult make_result(
	std::string_view fen,
	PerftResult stockfish_result,
	PerftResult tortoise_result)
{
	DivideResult result;
	result.fen = fen;
	result.stockfish_result = std::move(stockfish_result);
	result.tortoise_result = std::move(tortoise_result);
	return result;
}

std::optional<DivideResult> divide(std::string_view fen, int depth)
{
	Position position = Fen::load(fen);
	PerftResult stockfish_result = calc_stockfish_moves(fen, depth);
	PerftResult tortoise_result = calc_tortoise_moves(fen, depth);

	stockfish_result.sort_by_move();
	tortoise_result.sort_by_move();

	if (stockfish_result.moves.size() != tortoise_result.moves.size()) {
		return make_result(fen, std::move(stockfish_result), std::move(tortoise_result));
	}

	for (size_t i = 0; i < stockfish_result.moves.size(); i++) {
		const auto& stockfish_move_count = stockfish_result.moves[i];
		const auto& tortoise_move_count = tortoise_result.moves[i];
		const bool wrong_move = stockfish_move_count.move != tortoise_move_count.move;
		const bool wrong_count = stockfish_move_count.count != tortoise_move_count.count;

		if (wrong_move || (wrong_count && depth == 1)) {
			return make_result(fen, std::move(stockfish_result), std::move(tortoise_result));
		}
		if (wrong_count) {
			const Move move = long_notation_to_move(tortoise_move_count.move, position);
			position.make_move(move);
			return divide(Fen::save(position), depth - 1);
		}
	}
	return std::nullopt;
}

} // namespace tortoise
