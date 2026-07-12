#pragma once

#include "position.h"

#include <string>
#include <exception>
#include <utility>

namespace tortoise {
namespace Fen {

class BadFenException : public std::exception{};

extern const std::string initial_position;

std::string save(const Position& position);
bool load( const std::string& fen, Position& position);

Position load(const std::string_view fen);


struct EpdTest {
	Position position;
	std::string best_move;
	std::string description;
};

EpdTest load_epd_test(std::string_view epd);

} // namespace
} // namespace tortoise
