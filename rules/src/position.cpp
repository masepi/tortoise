#include "position.h"
#include "fen.h"
#include <iostream>

namespace tortoise {

Piece Position::get_piece(Square square) const
{
	const PieceType whiteType = get_piece_type<Color::white>(square);
	if (whiteType != PieceType::Empty) {
		return make_piece(whiteType, Color::white);
	}
	const PieceType blackType = get_piece_type<Color::black>(square);
	if (blackType != PieceType::Empty) {
		return make_piece(blackType, Color::black);
	}
	return Piece::Empty;
}

void Position::set_piece(Square square, Piece piece)
{
	if (PieceType t = white.get_piece_type(square); t != PieceType::Empty) {
		white.remove_piece(t, square);
	}
	if (PieceType t = black.get_piece_type(square); t != PieceType::Empty) {
		black.remove_piece(t, square);
	}
	if (piece == Piece::Empty) {
		return;
	}

	const PieceType type = tortoise::get_piece_type(piece);
	const Color color = tortoise::get_piece_color(piece);

	PiecesMap& pieces = ( color == Color::white ) ? white : black;
	pieces.put_piece(type, square);
}

} // namespace tortoise