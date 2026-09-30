#pragma once


#include "bitboard.h"
#include "types.h"

#include "arch.h"

namespace tortoise {



struct SliderEntry {
	const Bitboard* attacks;
	Bitboard mask;
};

extern SliderEntry rook_entries[square_count];
extern SliderEntry bishop_entries[square_count];

extern Bitboard rook_line_mask[square_count];
extern Bitboard bishop_line_mask[square_count];

inline Bitboard get_slider_hv_attack_map(Square square, Bitboard occ)
{
	const SliderEntry& e = rook_entries[static_cast<size_t>(square)];
	return e.attacks[pext(occ, e.mask)];
}

inline Bitboard get_slider_diagonal_attack_map(Square square, Bitboard occ)
{
	const SliderEntry& e = bishop_entries[static_cast<size_t>(square)];
	return e.attacks[pext(occ, e.mask)];
}

inline Bitboard get_slider_hv_attack_mask(Square square)
{
	return rook_line_mask[static_cast<size_t>(square)];
}

inline Bitboard get_slider_diagonal_attack_mask(Square square)
{
	return bishop_line_mask[static_cast<size_t>(square)];
}


////////////////////////////////////////////////////////////////////////////////////////////


const int attack_table_size = 256;

extern const Bitboard king_attack_map[square_count];
extern const Bitboard knight_attack_map[square_count];

extern const Bitboard slider_mask_h_map[square_count];
extern const Bitboard slider_attack_h_map[square_count][attack_table_size];

extern const Bitboard slider_mask_v_map[square_count];
extern const Bitboard slider_attack_v_map[square_count][attack_table_size];

extern const Bitboard slider_mask_diagonal1_map[square_count];
extern const Bitboard slider_attack_diagonal1_map[square_count][attack_table_size];

extern const Bitboard slider_mask_diagonal2_map[square_count];
extern const Bitboard slider_attack_diagonal2_map[square_count][attack_table_size];

extern const Bitboard black_pawn_attack_map[square_count];
extern const Bitboard white_pawn_attack_map[square_count];

extern const Bitboard en_passant_attack_by_map_up_left_map[square_count];
extern const Bitboard en_passant_attack_by_map_up_right_map[square_count];

extern const Bitboard extent_piece_attack_map[square_count][square_count];

template<Color color>
Bitboard get_king_attacked_by_pawn(Square king)
{
	return get_pawn_attack_map<color>(king);
}

extern bool enableDebug;

template<Direction direction>
constexpr inline Bitboard get_slider_attack_map(Square square, Bitboard occ)
{
	const int squareIndex = static_cast<int>(square);
	if constexpr (direction == Horizontal) {
		const size_t index = pext(occ, slider_mask_h_map[squareIndex]);
		return slider_attack_h_map[squareIndex][index];
	} else if constexpr (direction == Vertical) {
		const size_t index = pext(occ, slider_mask_v_map[squareIndex]);
		return slider_attack_v_map[squareIndex][index];
	} else if constexpr (direction == Diagonal_1) {
		const size_t index = pext(occ, slider_mask_diagonal1_map[squareIndex]);
		return slider_attack_diagonal1_map[squareIndex][index];
	} else if constexpr (direction == Diagonal_2) {
		const size_t index = pext(occ, slider_mask_diagonal2_map[squareIndex]);
		return slider_attack_diagonal2_map[squareIndex][index];
	}
}

inline Bitboard get_slider_diagonal_attack_map(Bitboard b, Bitboard occ)
{
	Bitboard result = 0;
	while (b) {
		const Square square = pop_lsb(b);
		result |= get_slider_diagonal_attack_map(square, occ);
	}
	return result;
}

inline Bitboard get_slider_hv_attack_map(Bitboard b, Bitboard occ)
{
	Bitboard result = 0;
	while (b) {
		const Square square = pop_lsb(b);
		result |= get_slider_hv_attack_map(square, occ);
	}
	return result;
}

template<Direction direction>
inline Bitboard get_slider_attack_map(Bitboard b, Bitboard occ)
{
	Bitboard result = 0;

	while (b) {
		const Square square = pop_lsb(b);
		result |= get_slider_attack_map<direction>(square, occ);
	}
	return result;
}

template<Color color, Direction direction>
inline Bitboard get_en_passant_attack_by(Square square)
{
	if constexpr ((color == Color::white && direction == UpLeft) ||
		          (color == Color::black && direction == UpRight)) {
		return en_passant_attack_by_map_up_right_map[static_cast<int>(square)];
	} else if constexpr ((color == Color::white && direction == UpRight) ||
		                 (color == Color::black && direction == UpLeft)) {
		return en_passant_attack_by_map_up_left_map[static_cast<int>(square)];
	}
}

inline Bitboard get_bishop_attack_map(Square square, Bitboard occ)
{
	return get_slider_diagonal_attack_map(square, occ);
}

inline Bitboard get_rook_attack_map(Square square, Bitboard occ)
{
	return get_slider_hv_attack_map(square, occ);
}

inline Bitboard get_knight_attack_map(Square square)
{
	return knight_attack_map[static_cast<int>(square)];
}

inline Bitboard get_knight_attack_map(Bitboard b)
{
	Bitboard result = 0;
	while (b) {
		const Square square = pop_lsb(b);
		result |= get_knight_attack_map(square);
	}
	return result;
}

inline Bitboard get_king_attack_map(Square square)
{
	return king_attack_map[static_cast<int>(square)];
}

inline Bitboard get_king_attack_map(Bitboard b)
{
	return get_king_attack_map(bitboard_to_square(b));
}

template<Color color>
inline Bitboard get_pawn_attack_map(Square square)
{
	if constexpr (color == Color::white) {
		return white_pawn_attack_map[static_cast<int>(square)];
	} else {
		return black_pawn_attack_map[static_cast<int>(square)];
	}
}

template<Color color>
inline Bitboard get_pawn_attack_map(Bitboard b)
{
	const Bitboard left = b & get_non_edge_file_map<color, Direction::UpLeft>();
	const Bitboard right = b & get_non_edge_file_map<color, Direction::UpRight>();

	return shift<color, Direction::UpLeft>(left) | shift<color, Direction::UpRight>(right);
}

template<PieceType type, Color color>
inline Bitboard get_attack_map(Square square, Bitboard occ)
{
	if constexpr (type == PieceType::Rook) {
		return get_rook_attack_map(square, occ);
	} else if constexpr (type == PieceType::Bishop) {
		return get_bishop_attack_map(square, occ);
	} else if constexpr (type == PieceType::Queen) {
		return get_rook_attack_map(square, occ) | get_bishop_attack_map(square, occ);
	} else if constexpr (type == PieceType::Knight) {
		return get_knight_attack_map(square);
	} else if constexpr (type == PieceType::Pawn) {
		return get_pawn_attack_map<color>(square);
	} else if constexpr (type == PieceType::King) {
		return get_king_attack_map(square);
	}
}

template<PieceType type, Color color>
inline Bitboard get_attack_map(Bitboard b, Bitboard occ)
{
	if constexpr(type==PieceType::King) {
		return get_king_attack_map(bitboard_to_square(b));
	}

	Bitboard attack_map = 0;
	while (b) {
		const Square square = pop_lsb(b);
		attack_map |= get_attack_map<type, color>(square, occ);
	}
	return attack_map;
}

inline Bitboard get_extent_piece_attack_map(Square piece, Square king)
{
	return extent_piece_attack_map[static_cast<int>(piece)][static_cast<int>(king)];
}

inline Bitboard extent_attack_map(Bitboard b, Square king)
{
	Bitboard result = 0;
	while (b) {
		const Square square = pop_lsb(b);
		result |= get_extent_piece_attack_map(square, king);
	}
	return result;
}

void create_attack_map();
void init_slider_attack_maps();
void init_king_attack_by_pawn();


class PieceAttackInitializer {
public:
	static void init();
private:
	PieceAttackInitializer();
};

} // namespace tortoise
