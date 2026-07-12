#pragma once
#include "types.h"
#include <array>

namespace tortoise {

enum class PromotionType {
	Knight,
	Bishop,
	Rook,
	Queen
};

inline PieceType make_promotion_piece_type(PromotionType type)
{
	switch (type) {
	case PromotionType::Knight:
		return PieceType::Knight;
	case PromotionType::Bishop:
		return PieceType::Bishop;
	case PromotionType::Rook:
		return PieceType::Rook;
	case PromotionType::Queen:
		return PieceType::Queen;
	}
	return PieceType::Queen;
}

inline Piece make_promotion_piece(PromotionType type, Color color)
{
	return make_piece(make_promotion_piece_type(type), color);
}

enum class MoveType {
	Normal,
	Castling,
	EnPassant,
	Promotion
};

enum class CastlingType {
	KingSide,
	QueenSide
};

typedef uint16_t MoveState;

// from - bits 0-5
// to - bits 6-11
// type - bits 12-13
// promotion type - bits 14-15
// castling type - determine by to() square
class Move {
public:
	// Do not initialize state due to performance issue.
	Move() {}
	
	static Move make_normal(Square from, Square to);
	static Move make_en_passant(Square from, Square to);
	static Move make_castling(Square from, Square to);
	static Move make_promotion(Square from, Square to, PromotionType promotion_type);

	Square from() const { return static_cast<Square>(state & 0x3F); }
	Square to() const { return static_cast<Square>((state >> 6) & 0x3F); }
	MoveType type() const { return static_cast<MoveType>((state >> 12) & 3); }
	PromotionType promotion() const { return static_cast<PromotionType>((state >> 14) & 3); }
	CastlingType get_castling_type() const { return get_file(to()) == File::FileG ? CastlingType::KingSide : CastlingType::QueenSide; }
 
	std::string to_string() const;

	bool operator==(const Move& other) const { return state == other.state; }
	bool operator<(const Move& other) const { return state < other.state; }

private:
	MoveState state;

	explicit Move(MoveState _state) : state(_state) {}
};

inline MoveState make_move_state(Square from, Square to, MoveType type = MoveType::Normal, PromotionType promotion = PromotionType::Knight)
{
	const uint32_t state =
		static_cast<uint32_t>(from) |
		(static_cast<uint32_t>(to) << 6) |
		(static_cast<uint32_t>(type) << 12) |
		(static_cast<uint32_t>(promotion) << 14);
	return static_cast<MoveState>(state);
}

inline Move Move::make_promotion(
	Square from, Square to, PromotionType promotion_type)
{
	return Move{make_move_state(from, to, MoveType::Promotion, promotion_type)};
}

inline Move Move::make_castling(Square from, Square to)
{
	return Move{make_move_state(from, to, MoveType::Castling)};
}

inline Move Move::make_normal(Square from, Square to)
{
	return Move{make_move_state(from, to)};
}

inline Move Move::make_en_passant(Square from, Square to)
{
	return Move{make_move_state(from, to, MoveType::EnPassant)};
}

inline std::string Move::to_string() const
{
	switch (type()) {
	case MoveType::Normal:
		return square_to_string(from()) + square_to_string(to());
	case MoveType::EnPassant:
		return file_to_string(get_file(from())) + "x" + square_to_string(to());
	case MoveType::Promotion:
	{
		std::string promotionPiece;
		switch (promotion()) {
		case PromotionType::Knight:
			promotionPiece = "N";
			break;
		case PromotionType::Bishop:
			promotionPiece = "B";
			break;
		case PromotionType::Rook:
			promotionPiece = "R";
			break;
		case PromotionType::Queen:
			promotionPiece = "Q";
			break;
		}
		return square_to_string(to()) + promotionPiece;
	}
	case MoveType::Castling:
	{
		switch (get_castling_type()) {
			case CastlingType::KingSide:
				return "O-O";
			case CastlingType::QueenSide:
				return "O-O-O";
		}
	}
	}
	return "";
}

class MoveList {
public:
	MoveList() : count(0) {}

	void push_back(Move move) { moves[count++] = move; }

	using MoveArray = std::array<Move, 220>;
	using iterator = MoveArray::iterator;
	using const_iterator = MoveArray::const_iterator;

	iterator begin() { return moves.begin(); }
	const_iterator begin() const { return moves.begin(); }

	iterator end() { return moves.begin() + static_cast<ptrdiff_t>(count); }
	const_iterator end() const { return moves.begin() + static_cast<ptrdiff_t>(count); }

	size_t size() const { return count; }
	bool empty() const { return count == 0; }
	void clear() { count = 0; }

	Move operator[](size_t index) const { return moves[index]; }
	Move& operator[](size_t index) { return moves[index]; }

private:
	MoveArray moves;
	size_t count;
};

} // namespace tortoise 