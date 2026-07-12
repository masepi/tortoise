#pragma once

#include <string>

#include "bitboard.h"

namespace tortoise {

class Position;

std::string format_ascii_position(const Position& pos);
void print_position(const Position& p);

void print_bitboard(Bitboard b,std::string_view msg = "");

} // namespace tortoise