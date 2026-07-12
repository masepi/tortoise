#pragma once

#include "bitboard.h"
#include "types.h"


namespace tortoise {
namespace Zobrist {

using Hash = uint64_t;

extern const Hash piece_hash_table[color_count][square_count][PieceTypeCount];
extern const Hash castling_rights_hash_table[CastlingRightsCount];
extern const Hash en_passant_hash_table[square_count+1];
extern const Hash initial_hash_value;
extern const Hash side_to_move_hash_table[2];

inline Hash initial_hash()
{
	return initial_hash_value;
}

inline Hash piece_hash(Color color, Square square, PieceType pieceType)
{
	return piece_hash_table[static_cast<int>(color)][static_cast<int>(square)][static_cast<int>(pieceType)];
}

inline Hash en_passant_hash(Square square)
{
	return en_passant_hash_table[static_cast<int>(square) + 1];
}

inline Hash en_passant_hash(Bitboard bitboard)
{
	const Square square = bitboard_to_square(bitboard);
	return en_passant_hash_table[static_cast<int>(square) + 1];
}

inline Hash castling_rights_hash(CastlingRights rights)
{
	return castling_rights_hash_table[static_cast<int>(rights)];
}

inline Hash side_to_move_hash(Color color)
{
	return side_to_move_hash_table[static_cast<int>(color)];
}

} // namespace Zobrist
} // namespace tortoise
