#include "debug.h"
#include <iostream>

#include "rules.h"

namespace tortoise {

std::string format_ascii_position(const Position& pos)
{
	std::string s;
	
	s += " +---+---+---+---+---+---+---+---+\n";
	for (int i = 0; i < 8; i++) {
		s += std::to_string(8 - i) + "| ";

		for (int j = 0; j < 8; j++) {
			const Piece piece = pos.get_piece(static_cast<Square>((7 - i) * 8 + j));

			if (get_piece_color(piece) == Color::black) {
				switch (get_piece_type(piece)) {
				case PieceType::Pawn:
					s += 'p';
					break;
				case PieceType::Knight:
					s += 'n';
					break;
				case PieceType::Bishop:
					s += 'b';
					break;
				case PieceType::Rook:
					s += 'r';
					break;
				case PieceType::Queen:
					s += 'q';
					break;
				case PieceType::King:
					s += 'k';
					break;
				case PieceType::Empty:
					s += ' ';
					break;
				}
			} else {
				switch (get_piece_type(piece)) {
				case PieceType::Pawn:
					s += 'P';
					break;
				case PieceType::Knight:
					s += 'N';
					break;
				case PieceType::Bishop:
					s += 'B';
					break;
				case PieceType::Rook:
					s += 'R';
					break;
				case PieceType::Queen:
					s += 'Q';
					break;
				case PieceType::King:
					s += 'K';
					break;
				case PieceType::Empty:
					s += ' ';
					break;
				}
			}

			s += " | ";
		}
		s += L'\n';
		s += " +---+---+---+---+---+---+---+---+\n";
	}
	s += "   A   B   C   D   E   F   G   H  \n";
	return s;
}

void print_position(const Position& p)
{
	std::cout << format_ascii_position(p) << std::endl;
}

void print_bitboard(Bitboard b, std::string_view msg)
{
	std::string s;

	if (!msg.empty()) {
		s += msg;
		s += "\n";
	}

	for (int i = 7; i >= 0; i--) {
		for (int j = 0; j < 8; j++) {
			const uint64_t bit = 1ULL << (i * 8 + j);
			if ((b & bit) != 0) {
				s += "X";
			} else {
				s += ".";
			}
		}
		s += "\n";
	}
	s += "\n";
	std::cout << s;
	std::cout.flush();
}

} // namespace tortoise