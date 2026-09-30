#pragma once

#include <array>
#include <assert.h>
#include <bit>
#include <stdint.h>
#include <string>

#include "types.h"
#include "bitboard.h"
#include "debug.h"
#include "move.h"
#include "piece_attack.h"
#include "zobrist.h"
#include "arch.h"

namespace tortoise {

struct PositionInfo {
	Bitboard non_pin;

	Bitboard non_pin_h;
	Bitboard non_pin_v;
	Bitboard non_pin_diagonal1;
	Bitboard non_pin_diagonal2;

	Bitboard occupied;
	Bitboard occupied_black;
	Bitboard occupied_white;
	Bitboard unoccupied;
	Bitboard unoccupied_or_opponent_occupied;

	Bitboard attacked;
	Bitboard blockers;
	Bitboard checkers;

	Bitboard promotion_candidates;

	MoveList* legal_moves;

	PositionInfo() :
		non_pin(0),
		non_pin_h(0),
		non_pin_v(0),
		non_pin_diagonal1(0),
		non_pin_diagonal2(0),
		occupied(0),
		occupied_black(0),
		occupied_white(0),
		unoccupied(0),
		unoccupied_or_opponent_occupied(0),
		attacked(0),
		blockers(0),
		checkers(0),
		promotion_candidates(0),
		legal_moves(nullptr)
	{}

	template<Direction direction>
	inline constexpr Bitboard GetNonPin() const
	{
		if constexpr (direction == Left) {
			return non_pin_h;
		} else if constexpr (direction == Right) {
			return non_pin_h;
		} else if constexpr (direction == Down) {
			return non_pin_v;
		} else if constexpr (direction == Up) {
			return non_pin_v;
		} else if constexpr (direction == UpLeft) {
			return non_pin_diagonal1;
		} else if constexpr (direction == UpRight) {
			return non_pin_diagonal2;
		} else if constexpr (direction == DownLeft) {
			return non_pin_diagonal2;
		} else if constexpr (direction == DownRight) {
			return non_pin_diagonal1;
		}
	}

	template<Color color>
	constexpr Bitboard GetOccupied() const
	{
		if constexpr (color == Color::white) {
			return occupied_white;
		} else {
			return occupied_black;
		}
	}

	template<Color color>
	constexpr Bitboard GetOpponentOccupied() const
	{
		if constexpr (color == Color::white) {
			return occupied_black;
		} else {
			return occupied_white;
		}
	}

	inline bool HasPinned() const { return non_pin != 0xFFFFFFFFFFFFFFFFull; }
};


struct PiecesMap {
	Bitboard maps[7];
	int8_t   board[64];
	
	PiecesMap() :
		maps{0},
		board{0}
	{}

	Bitboard Pawn() const { return maps[static_cast<size_t>(PieceType::Pawn)]; }
	Bitboard Knight() const { return maps[static_cast<size_t>(PieceType::Knight)]; }
	Bitboard Bishop() const { return maps[static_cast<size_t>(PieceType::Bishop)]; }
	Bitboard Rook() const { return maps[static_cast<size_t>(PieceType::Rook)]; }
	Bitboard Queen() const { return maps[static_cast<size_t>(PieceType::Queen)]; }
	Bitboard King() const { return maps[static_cast<size_t>(PieceType::King)]; }

	PieceType get_piece_type(Square s) const {
		return static_cast<PieceType>(board[static_cast<size_t>(s)]);
	}

	void put_piece(PieceType t, Square s) {
		maps[static_cast<size_t>(t)] |= square_to_bitboard(s);
		board[static_cast<size_t>(s)] = static_cast<int8_t>(t);
	}
	void move_piece(PieceType t, Square from, Square to) {
		maps[static_cast<size_t>(t)] ^= square_to_bitboard(from) | square_to_bitboard(to);
		board[static_cast<size_t>(from)] = 0;
		board[static_cast<size_t>(to)]   = static_cast<int8_t>(t);
	}
	void remove_piece(PieceType t, Square s) {
		maps[static_cast<size_t>(t)] &= ~square_to_bitboard(s);
		board[static_cast<size_t>(s)] = 0;
	}

	template<PieceType piece_type>
	constexpr Bitboard get_map() const { return maps[static_cast<size_t>(piece_type)]; }
	Bitboard& get_map(PieceType piece_type) { return maps[static_cast<size_t>(piece_type)]; }
	
	bool operator==(const PiecesMap& other) const = default;
};



enum class GameState {
	in_game, 
	draw, 
	black_win,
	white_win 
};

class Position {
public:
	Position() :
		enPassant(0),
		zobristHash(Zobrist::initial_hash()),
		sideToMove(Color::white),
		castlingRights(CastlingRights::All),
		rule50(0),
		fullMoveNumber(0)
	{}

	GameState analyze(MoveList& moves) const;
	void analyze_moves(MoveList& moves) const;

	template<bool UpdateZobrist = true>
	void make_move(Move move);

	void init();

	Color side_to_move() const { return sideToMove; }
	Color us() const { return sideToMove; }
	Color them() const { return get_opponent_color(sideToMove); }

	Piece get_piece(Square square) const;
	void set_piece(Square square, Piece piece);

	void set_side_to_move(Color color) { sideToMove = color; }

	template<Color color>
	PieceType get_piece_type(Square square) const;

	PieceType get_piece_type(Square square) const;
	PieceType get_opponent_piece_type(Square square) const;

	void set_en_passant(Square square) { enPassant = square == Square::SquareNone ? 0 : square_to_bitboard(square); }
	Square get_en_passant() const { return enPassant == 0 ? Square::SquareNone : bitboard_to_square(enPassant); }

	CastlingRights get_castling_rights() const { return castlingRights; }
	void set_castling_rights(CastlingRights rights) { castlingRights = rights; }

	int get_rule50() const { return rule50; }
	void set_rule50(int moves) { rule50 = moves; }

	int get_full_move_number() const { return fullMoveNumber; }
	void set_full_move_number(int moves) { fullMoveNumber = moves; }

	Zobrist::Hash hash_key() const { return zobristHash; }

	bool operator==(const Position& other) const;

	template<Color color>
	const PiecesMap& get_map() const
	{
		if constexpr (color == Color::white) {
			return white;
		} else {
			return black;
		}
	}

private:
	PiecesMap white;
	PiecesMap black;
	Bitboard enPassant;
	Bitboard zobristHash;

	Color sideToMove;
	CastlingRights castlingRights;
	int rule50;
	int fullMoveNumber;

	template<Color color>
	GameState do_analyze(MoveList& moves) const;

	template<Color color>
	void do_analyze_moves(MoveList& moves) const;

	template<Color color>
	void create_info(PositionInfo& info) const;

	template<Color color>
	const PiecesMap& get_pieces() const;

	template<Color color>
	PiecesMap& get_pieces();

	template<Color color>
	const PiecesMap& get_opponent_pieces() const;

	template<Color color>
	PiecesMap& get_opponent_pieces();

	enum class GenerationState { Normal, Evasions, KingEvasions };

	template<Color color, GenerationState state>
	void generate_moves(const PositionInfo& info) const;

	template<Color color, GenerationState state, bool HasPinned>
	void generate_moves(const PositionInfo& info) const;

	template<Color color, GenerationState state, bool HasPinned>
	void generate_pawn_regular_move(const PositionInfo& info) const;

	template<Color color, Direction direction, GenerationState state, bool HasPinned>
	void generate_pawn_capture_move(const PositionInfo& info) const;

	template<Color color, Direction direction, GenerationState state, bool HasPinned>
	void generate_pawn_en_passant_move(const PositionInfo& info) const;

	template<Color color, GenerationState state, bool HasPinned>
	void generate_pawn_promotion_move(const PositionInfo& info) const;

	template<Color color, Direction direction, GenerationState state, bool HasPinned>
	void generate_pawn_capture_promotion_move(const PositionInfo& info) const;

	template<Color color, GenerationState state, bool HasPinned>
	void generate_knight_move(const PositionInfo& info) const;

	template<Color color, GenerationState state>
	void generate_king_move(const PositionInfo& info) const;

	template<Color color, CastlingType type, GenerationState state>
	void generate_castling_move(const PositionInfo& info) const;

	template<Color color, Direction direction, GenerationState state>
	void generate_slider_move(const PositionInfo& info) const;

	template<Color color, Position::GenerationState state>
	void generate_slider_diagonal_non_pin_move(const PositionInfo& info) const;

	template<Color color, Position::GenerationState state>
	void generate_slider_hv_non_pin_move(const PositionInfo& info) const;

	template<Color color, bool UpdateZobristHash>
	void do_make_move(Move move);

	template<Color color, bool UpdateZobristHash>
	void do_castling(CastlingType castlingType);

	template<Color color, bool UpdateZobristHash>
	void do_promotion(PromotionType promotionType, Square from, Square to);

	template<Color color, bool UpdateZobristHash>
	void do_en_passant(Square from, Square to);

	template<Color color, bool UpdateZobristHash>
	void do_normal(Square from, Square to);

	void recalc_zobrist_hash();

	void update_castling_rights(Square from, Square to);

	template<Color color, GenerationState state>
	void append_move(const PositionInfo& info, Square from, Square to) const;

	template<Color color, GenerationState state>
	void append_king_move(const PositionInfo& info, Square from, Square to) const;

	template<Color color, GenerationState state>
	void append_promotion_moves(const PositionInfo& info, Square from, Square to) const;

	template<Color color, GenerationState state>
	void append_en_passant_move(const PositionInfo& info, Square from, Square to) const;

	template<Color color, GenerationState state, CastlingType castlingType>
	void append_castling_move(const PositionInfo& info) const;
};

inline GameState Position::analyze(MoveList& moves) const
{
	moves.clear();
	if (side_to_move() == Color::white) {
		return do_analyze<Color::white>(moves);
	}
	return do_analyze<Color::black>(moves);
}

inline void Position::analyze_moves(MoveList& moves) const
{
	moves.clear();
	if (side_to_move() == Color::white) {
		do_analyze_moves<Color::white>(moves);
	} else {
		do_analyze_moves<Color::black>(moves);
	}
}

template<Color color>
GameState Position::do_analyze(MoveList& moves) const
{
	PositionInfo info;
	info.legal_moves = &moves;
	create_info<color>(info);

	if (info.checkers) {
		if (info.blockers) {
			generate_moves<color, GenerationState::Evasions>(info);
		} else {
			generate_king_move<color, GenerationState::KingEvasions>(info);
		}
	} else {
		generate_moves<color, GenerationState::Normal>(info);
	}

	if (info.legal_moves->empty()) {
		if (info.checkers) {
			if constexpr (color == Color::white) {
				return GameState::black_win;
			} else {
				return GameState::white_win;
			}
		}
		return GameState::draw;
	}

	if (rule50 >= 100) {
		return GameState::draw;
	}

	const Bitboard pawns_rooks_queens = white.Pawn() | black.Pawn() | white.Rook() | black.Rook() | white.Queen() | black.Queen();
	if (pawns_rooks_queens == 0) {
		const int minor_count = std::popcount(white.Knight() | black.Knight() | white.Bishop() | black.Bishop());
		if (minor_count <= 1) {
			return GameState::draw;
		}

		if ((white.Knight() | black.Knight()) == 0) {
			constexpr Bitboard one_square_color = 0x55AA55AA55AA55AAull;
			const Bitboard bishops = white.Bishop() | black.Bishop();
			if ((bishops & one_square_color) == 0 || (bishops & ~one_square_color) == 0) {
				return GameState::draw;
			}
		}
	}

	return GameState::in_game;
}

template<Color color>
FORCE_INLINE void Position::do_analyze_moves(MoveList& moves) const
{
	PositionInfo info;
	info.legal_moves = &moves;
	create_info<color>(info);

	if (info.checkers) {
		if (info.blockers) {
			generate_moves<color, GenerationState::Evasions>(info);
		} else {
			generate_king_move<color, GenerationState::KingEvasions>(info);
		}
	} else {
		generate_moves<color, GenerationState::Normal>(info);
	}
}

template<Color color>
FORCE_INLINE void Position::create_info(PositionInfo& info) const
{
	constexpr const Color opponent_color = get_opponent_color<color>();
	const PiecesMap& pieces = get_pieces<color>();
	const PiecesMap& opponentPieces = get_opponent_pieces<color>();

	info.occupied_black =
		black.Pawn() | black.Knight() | black.Bishop() | black.Rook() | black.Queen() | black.King();
	info.occupied_white =
		white.Pawn() | white.Knight() | white.Bishop() | white.Rook() | white.Queen() | white.King();
	info.occupied = info.occupied_black | info.occupied_white;
	info.unoccupied = ~info.occupied;
	info.unoccupied_or_opponent_occupied = info.unoccupied | info.GetOpponentOccupied<color>();

	info.promotion_candidates = pieces.Pawn() & get_promotion_start_point_map<color>();

	const Bitboard pawnAttack = get_pawn_attack_map<opponent_color>(opponentPieces.Pawn());
	const Bitboard knightAttack = get_knight_attack_map(opponentPieces.Knight());
	const Bitboard kingAttack = get_king_attack_map(opponentPieces.King());

	const Bitboard opponentDiagonalSliders = opponentPieces.Queen() | opponentPieces.Bishop();
	const Bitboard opponentHVSliders = opponentPieces.Queen() | opponentPieces.Rook();

	const Square kingSquare = bitboard_to_square(pieces.King());

	const bool hasHVsliders = opponentHVSliders & get_slider_hv_attack_mask(kingSquare);
	const bool hasDiagonalSliders =
		opponentDiagonalSliders & get_slider_diagonal_attack_mask(kingSquare);

	if (!hasHVsliders && !hasDiagonalSliders) {
		info.non_pin = ~0ull;
		info.non_pin_h = ~0ull;
		info.non_pin_v = ~0ull;
		info.non_pin_diagonal1 = ~0ull;
		info.non_pin_diagonal2 = ~0ull;

		const Bitboard slidersDiagonalAttack =
			get_slider_diagonal_attack_map(opponentDiagonalSliders, info.occupied);
		const Bitboard slidersHVAttack = get_slider_hv_attack_map(opponentHVSliders, info.occupied);

		info.attacked =
			pawnAttack | knightAttack | kingAttack | slidersDiagonalAttack | slidersHVAttack;

		const bool isCheck = pieces.King() & info.attacked;
		if (isCheck) {
			const Bitboard kingByPawnAttack =
				opponentPieces.Pawn() & get_king_attacked_by_pawn<color>(kingSquare);
			const Bitboard kingByKnightAttack =
				opponentPieces.Knight() & get_knight_attack_map(pieces.King());

			info.checkers = kingByPawnAttack | kingByKnightAttack;
			const int checkerCount = std::popcount(info.checkers);
			if (checkerCount == 1) {
				info.blockers = info.checkers;
			} else {
				info.blockers = 0;
			}
		} else {
			info.blockers = 0;
			info.checkers = 0;
		}
	} else if (!hasDiagonalSliders) {
		const Bitboard attackDiagonal =
			get_slider_diagonal_attack_map(opponentDiagonalSliders, info.occupied);
		const Bitboard attackH =
			get_slider_attack_map<Horizontal>(opponentHVSliders, info.occupied);
		const Bitboard attackV = get_slider_attack_map<Vertical>(opponentHVSliders, info.occupied);

		info.attacked = pawnAttack | knightAttack | kingAttack | attackDiagonal | attackH | attackV;

		const Bitboard kingSliderHorizontal =
			get_slider_attack_map<Horizontal>(kingSquare, info.occupied);
		const Bitboard kingSliderVertical =
			get_slider_attack_map<Vertical>(kingSquare, info.occupied);

		const Bitboard kingAttackedByHorizontal =
			kingSliderHorizontal & (attackH | opponentHVSliders);
		const Bitboard kingAttackedByVertical = kingSliderVertical & (attackV | opponentHVSliders);

		const Bitboard pinH = kingAttackedByHorizontal & info.GetOccupied<color>();
		const Bitboard pinV = kingAttackedByVertical & info.GetOccupied<color>();

		info.non_pin_h = ~pinV;
		info.non_pin_v = ~pinH;
		info.non_pin = info.non_pin_h & info.non_pin_v;
		info.non_pin_diagonal1 = info.non_pin;
		info.non_pin_diagonal2 = info.non_pin;

		const bool isCheck = pieces.King() & info.attacked;
		if (isCheck) {
			const Bitboard kingByPawnAttack =
				opponentPieces.Pawn() & get_king_attacked_by_pawn<color>(kingSquare);
			const Bitboard kingByKnightAttack =
				opponentPieces.Knight() & get_knight_attack_map(pieces.King());

			const Bitboard sliderCheckers =
				(kingAttackedByVertical | kingAttackedByHorizontal) & opponentHVSliders;
			const Bitboard kingByPawnAndKnight = kingByPawnAttack | kingByKnightAttack;

			info.checkers = kingByPawnAndKnight | sliderCheckers;
			const int checkerCount = std::popcount(info.checkers);
			if (checkerCount == 1) {
				const Square checker = bitboard_to_square(info.checkers);
				info.blockers = get_between_map(kingSquare, checker) | kingByPawnAndKnight;
			} else {
				info.blockers = 0;
			}
			info.attacked |= extent_attack_map(sliderCheckers, kingSquare);
		} else {
			info.blockers = 0;
			info.checkers = 0;
		}
	} else if (!hasHVsliders) {
		const Bitboard attackDiagonal1 =
			get_slider_attack_map<Diagonal_1>(opponentDiagonalSliders, info.occupied);
		const Bitboard attackDiagonal2 =
			get_slider_attack_map<Diagonal_2>(opponentDiagonalSliders, info.occupied);
		const Bitboard attackHV = get_slider_hv_attack_map(opponentHVSliders, info.occupied);

		info.attacked =
			pawnAttack | knightAttack | kingAttack | attackDiagonal1 | attackDiagonal2 | attackHV;

		const Bitboard kingSliderDiagonal1 =
			get_slider_attack_map<Diagonal_1>(kingSquare, info.occupied);
		const Bitboard kingSliderDiagonal2 =
			get_slider_attack_map<Diagonal_2>(kingSquare, info.occupied);

		const Bitboard kingAttackedByDiagonal1 =
			kingSliderDiagonal1 & (attackDiagonal1 | opponentDiagonalSliders);
		const Bitboard kingAttackedByDiagonal2 =
			kingSliderDiagonal2 & (attackDiagonal2 | opponentDiagonalSliders);

		const Bitboard pinDiagonal1 = kingAttackedByDiagonal1 & info.GetOccupied<color>();
		const Bitboard pinDiagonal2 = kingAttackedByDiagonal2 & info.GetOccupied<color>();

		info.non_pin_diagonal1 = ~pinDiagonal2;
		info.non_pin_diagonal2 = ~pinDiagonal1;

		info.non_pin = info.non_pin_diagonal1 & info.non_pin_diagonal2;
		info.non_pin_h = info.non_pin;
		info.non_pin_v = info.non_pin;

		const bool isCheck = pieces.King() & info.attacked;
		if (isCheck) {
			const Bitboard kingByPawnAttack =
				opponentPieces.Pawn() & get_king_attacked_by_pawn<color>(kingSquare);
			const Bitboard kingByKnightAttack =
				opponentPieces.Knight() & get_knight_attack_map(pieces.King());

			const Bitboard sliderCheckers =
				(kingAttackedByDiagonal1 | kingAttackedByDiagonal2) & opponentDiagonalSliders;
			const Bitboard kingByPawnAndKnight = kingByPawnAttack | kingByKnightAttack;

			info.checkers = kingByPawnAndKnight | sliderCheckers;

			const int checkerCount = std::popcount(info.checkers);
			if (checkerCount == 1) {
				const Square checker = bitboard_to_square(info.checkers);
				info.blockers = get_between_map(kingSquare, checker) | kingByPawnAndKnight;
			} else {
				info.blockers = 0;
			}
			info.attacked |= extent_attack_map(sliderCheckers, kingSquare);
		} else {
			info.blockers = 0;
			info.checkers = 0;
		}
	} else {
		const Bitboard attackDiagonal1 =
			get_slider_attack_map<Diagonal_1>(opponentDiagonalSliders, info.occupied);
		const Bitboard attackDiagonal2 =
			get_slider_attack_map<Diagonal_2>(opponentDiagonalSliders, info.occupied);
		const Bitboard attackH =
			get_slider_attack_map<Horizontal>(opponentHVSliders, info.occupied);
		const Bitboard attackV = get_slider_attack_map<Vertical>(opponentHVSliders, info.occupied);

		info.attacked = pawnAttack | knightAttack | kingAttack | attackDiagonal1 | attackDiagonal2 |
						attackH | attackV;

		const Bitboard kingSliderHorizontal =
			get_slider_attack_map<Horizontal>(kingSquare, info.occupied);
		const Bitboard kingSliderVertical =
			get_slider_attack_map<Vertical>(kingSquare, info.occupied);
		const Bitboard kingSliderDiagonal1 =
			get_slider_attack_map<Diagonal_1>(kingSquare, info.occupied);
		const Bitboard kingSliderDiagonal2 =
			get_slider_attack_map<Diagonal_2>(kingSquare, info.occupied);

		const Bitboard kingAttackedByHorizontal =
			kingSliderHorizontal & (attackH | opponentHVSliders);
		const Bitboard kingAttackedByVertical = kingSliderVertical & (attackV | opponentHVSliders);
		const Bitboard kingAttackedByDiagonal1 =
			kingSliderDiagonal1 & (attackDiagonal1 | opponentDiagonalSliders);
		const Bitboard kingAttackedByDiagonal2 =
			kingSliderDiagonal2 & (attackDiagonal2 | opponentDiagonalSliders);

		const Bitboard pinH = kingAttackedByHorizontal & info.GetOccupied<color>();
		const Bitboard pinV = kingAttackedByVertical & info.GetOccupied<color>();
		const Bitboard pinDiagonal1 = kingAttackedByDiagonal1 & info.GetOccupied<color>();
		const Bitboard pinDiagonal2 = kingAttackedByDiagonal2 & info.GetOccupied<color>();

		const Bitboard pinDiagonal12 = pinDiagonal1 | pinDiagonal2;
		const Bitboard pinHV = pinH | pinV;

		info.non_pin = ~(pinDiagonal12 | pinHV);

		info.non_pin_h = ~(pinV | pinDiagonal12);
		info.non_pin_v = ~(pinH | pinDiagonal12);
		info.non_pin_diagonal1 = ~(pinDiagonal2 | pinHV);
		info.non_pin_diagonal2 = ~(pinDiagonal1 | pinHV);

		const bool isCheck = pieces.King() & info.attacked;
		if (isCheck) {
			const Bitboard kingByPawnAttack =
				opponentPieces.Pawn() & get_king_attacked_by_pawn<color>(kingSquare);
			const Bitboard kingByKnightAttack =
				opponentPieces.Knight() &
				get_attack_map<PieceType::Knight, color>(pieces.King(), info.occupied);
			const Bitboard kingByHorizontal = kingAttackedByHorizontal & opponentHVSliders;
			const Bitboard kingByVertical = kingAttackedByVertical & opponentHVSliders;
			const Bitboard kingByDiagonal1 = kingAttackedByDiagonal1 & opponentDiagonalSliders;
			const Bitboard kingByDiagonal2 = kingAttackedByDiagonal2 & opponentDiagonalSliders;

			const Bitboard sliderCheckers =
				kingByHorizontal | kingByVertical | kingByDiagonal1 | kingByDiagonal2;
			const Bitboard kingByPawnAndKnight = kingByPawnAttack | kingByKnightAttack;

			info.checkers = kingByPawnAndKnight | sliderCheckers;
			const int checkerCount = std::popcount(info.checkers);
			if (checkerCount == 1) {
				const Square checker = bitboard_to_square(info.checkers);
				info.blockers = get_between_map(kingSquare, checker) | kingByPawnAndKnight;
			} else {
				info.blockers = 0;
			}
			info.attacked |= extent_attack_map(sliderCheckers, kingSquare);
		} else {
			info.blockers = 0;
			info.checkers = 0;
		}
	}
}

template<Color color>
inline PieceType Position::get_piece_type(Square square) const
{
	return get_pieces<color>().get_piece_type(square);
}

inline PieceType Position::get_piece_type(Square square) const
{
	if (side_to_move() == Color::white) {
		return get_piece_type<Color::white>(square);
	} else {
		return get_piece_type<Color::black>(square);
	}
}

inline PieceType Position::get_opponent_piece_type(Square square) const
{
	if (side_to_move() != Color::white) {
		return get_piece_type<Color::white>(square);
	} else {
		return get_piece_type<Color::black>(square);
	}
}

template<Color color>
inline const PiecesMap& Position::get_pieces() const
{
	if constexpr (color == Color::white) {
		return white;
	} else {
		return black;
	}
}

template<Color color>
inline PiecesMap& Position::get_pieces()
{
	if constexpr (color == Color::white) {
		return white;
	} else {
		return black;
	}
}

template<Color color>
const PiecesMap& Position::get_opponent_pieces() const
{
	if constexpr (color == Color::white) {
		return black;
	} else {
		return white;
	}
}

template<Color color>
PiecesMap& Position::get_opponent_pieces()
{
	if constexpr (color == Color::white) {
		return black;
	} else {
		return white;
	}
}

template<Color color, Position::GenerationState state>
FORCE_INLINE void Position::generate_moves(const PositionInfo& info) const
{
	if (info.HasPinned()) {
		generate_moves<color, state, true>(info);
	} else {
		generate_moves<color, state, false>(info);
	}
}

template<Color color, Position::GenerationState state, bool HasPinned>
FORCE_INLINE void Position::generate_moves(const PositionInfo& info) const
{
	generate_pawn_regular_move<color, state, HasPinned>(info);
	generate_pawn_capture_move<color, UpLeft, state, HasPinned>(info);
	generate_pawn_capture_move<color, UpRight, state, HasPinned>(info);

	if (enPassant != 0) {
		generate_pawn_en_passant_move<color, UpLeft, state, HasPinned>(info);
		generate_pawn_en_passant_move<color, UpRight, state, HasPinned>(info);
	}

	if (info.promotion_candidates) {
		generate_pawn_promotion_move<color, state, HasPinned>(info);
		generate_pawn_capture_promotion_move<color, UpLeft, state, HasPinned>(info);
		generate_pawn_capture_promotion_move<color, UpRight, state, HasPinned>(info);
	}

	generate_king_move<color, state>(info);

	generate_castling_move<color, CastlingType::KingSide, state>(info);
	generate_castling_move<color, CastlingType::QueenSide, state>(info);

	generate_knight_move<color, state, HasPinned>(info);

	if constexpr (HasPinned) {
		generate_slider_move<color, Diagonal_1, state>(info);
		generate_slider_move<color, Diagonal_2, state>(info);

		generate_slider_move<color, Horizontal, state>(info);
		generate_slider_move<color, Vertical, state>(info);
	} else {
		generate_slider_diagonal_non_pin_move<color, state>(info);
		generate_slider_hv_non_pin_move<color, state>(info);
	}
}

template<Color color, Position::GenerationState state, bool HasPinned>
FORCE_INLINE void Position::generate_pawn_regular_move(const PositionInfo& info) const
{
	const PiecesMap& pieces = get_pieces<color>();

	Bitboard upMapFrom = pieces.Pawn() & get_non_promotion_map<color>();

	if constexpr (HasPinned) {
		upMapFrom &= info.GetNonPin<Up>();
	}

	Bitboard upMapTo = info.unoccupied & shift<color, Up>(upMapFrom);
	Bitboard longUpTo =
		info.unoccupied & get_pawn_long_jump_finish_map<color>() & shift<color, Up>(upMapTo);

	if constexpr (state == Position::GenerationState::Evasions) {
		upMapTo &= info.blockers;
		longUpTo &= info.blockers;
	}

	while (upMapTo) {
		const Square to = pop_lsb(upMapTo);
		const Square from = shift<color, Down>(to);
		append_move<color, state>(info, from, to);
	}

	while (longUpTo) {
		const Square to = pop_lsb(longUpTo);
		const Square from = shift<color, Down, Down>(to);
		append_move<color, state>(info, from, to);
	}
}

template<Color color, Direction direction, Position::GenerationState state, bool HasPinned>
FORCE_INLINE void Position::generate_pawn_capture_move(const PositionInfo& info) const
{
	const PiecesMap& pieces = get_pieces<color>();

	Bitboard fromMap =
		pieces.Pawn() & get_non_promotion_map<color>() & get_non_edge_file_map<color, direction>();

	if constexpr (HasPinned) {
		fromMap &= info.GetNonPin<direction>();
	}

	Bitboard toMap = info.GetOpponentOccupied<color>() & shift<color, direction>(fromMap);

	if constexpr (state == Position::GenerationState::Evasions) {
		toMap &= info.blockers;
	}

	while (toMap) {
		const Square to = pop_lsb(toMap);
		const Square from = shift_invert<color, direction>(to);
		append_move<color, state>(info, from, to);
	}
}

template<Color color, Direction direction, Position::GenerationState state, bool HasPinned>
FORCE_INLINE void Position::generate_pawn_en_passant_move(const PositionInfo& info) const
{
	if constexpr (state == Position::GenerationState::Evasions) {
		const Bitboard checker_pawn = get_en_passant_finish_point<color>(enPassant);
		if ((checker_pawn & info.checkers) == 0) {
			return;
		}
	}

	const PiecesMap& pieces = get_pieces<color>();
	const Square to = bitboard_to_square(enPassant);

	Bitboard pawnsFrom = get_en_passant_attack_by<color, direction>(to) & pieces.Pawn();

	if constexpr (HasPinned) {
		pawnsFrom &= info.GetNonPin<direction>();
	}

	while (pawnsFrom) {
		const Square from = pop_lsb(pawnsFrom);
		append_en_passant_move<color, state>(info, from, to);
	}
}

template<Color color, Position::GenerationState state, bool HasPinned>
inline void Position::generate_pawn_promotion_move(const PositionInfo& info) const
{
	Bitboard pawnsFrom = info.promotion_candidates;

	if constexpr (HasPinned) {
		pawnsFrom &= info.GetNonPin<Up>();
	}

	Bitboard pawnsTo =
		shift<color, Up>(pawnsFrom) & get_promotion_finish_point_map<color>() & info.unoccupied;

	if constexpr (state == Position::GenerationState::Evasions) {
		pawnsTo &= info.blockers;
	}

	while (pawnsTo) {
		const Square to = pop_lsb(pawnsTo);
		const Square from = shift<color, Down>(to);
		append_promotion_moves<color, state>(info, from, to);
	}
}

template<Color color, Direction direction, Position::GenerationState state, bool HasPinned>
inline void Position::generate_pawn_capture_promotion_move(const PositionInfo& info) const
{
	Bitboard pawnsFrom = info.promotion_candidates & get_non_edge_file_map<color, direction>();

	if constexpr (HasPinned) {
		pawnsFrom &= info.GetNonPin<direction>();
	}

	Bitboard pawnsTo = shift<color, direction>(pawnsFrom) & info.GetOpponentOccupied<color>();

	if constexpr (state == Position::GenerationState::Evasions) {
		pawnsTo &= info.blockers;
	}

	while (pawnsTo) {
		const Square to = pop_lsb(pawnsTo);
		const Square from = shift_invert<color, direction>(to);
		append_promotion_moves<color, state>(info, from, to);
	}
}

template<Color color, Position::GenerationState state, bool HasPinned>
inline void Position::generate_knight_move(const PositionInfo& info) const
{
	const PiecesMap& pieces = get_pieces<color>();
	Bitboard fromMap = pieces.Knight();

	if constexpr (HasPinned) {
		fromMap &= info.non_pin;
	}

	while (fromMap) {
		const Square from = pop_lsb(fromMap);
		Bitboard attackMap = get_knight_attack_map(from) & info.unoccupied_or_opponent_occupied;

		if constexpr (state == Position::GenerationState::Evasions) {
			attackMap &= info.blockers;
		}

		while (attackMap) {
			const Square to = pop_lsb(attackMap);
			append_move<color, state>(info, from, to);
		}
	}
}

template<Color color, Position::GenerationState state>
FORCE_INLINE void Position::generate_king_move(const PositionInfo& info) const
{
	const PiecesMap& pieces = get_pieces<color>();
	const Square from = bitboard_to_square(pieces.King());

	Bitboard attackMap =
		get_king_attack_map(from) & ~info.attacked & info.unoccupied_or_opponent_occupied;

	while (attackMap) {
		const Square to = pop_lsb(attackMap);
		append_king_move<color, state>(info, from, to);
	}
}

template<Color color, CastlingType type, Position::GenerationState state>
inline void Position::generate_castling_move(const PositionInfo& info) const
{
	if constexpr (state != GenerationState::Normal) {
		return;
	}

	if constexpr (color == Color::white) {
		if constexpr (type == CastlingType::KingSide) {
			if ((castlingRights & CastlingRights::White_O_O) != CastlingRights::None) {
				if ((info.occupied & white_king_side_castling_occ_map) == 0 &&
					(info.attacked & white_king_side_castling_attack_map) == 0)
				{
					append_castling_move<color, state, CastlingType::KingSide>(info);
				}
			}
		} else {
			if ((castlingRights & CastlingRights::White_O_O_O) != CastlingRights::None) {
				if ((info.occupied & white_queen_side_castling_occ_map) == 0 &&
					(info.attacked & white_queen_side_castling_attack_map) == 0)
				{
					append_castling_move<color, state, CastlingType::QueenSide>(info);
				}
			}
		}
	} else {
		if constexpr (type == CastlingType::KingSide) {
			if (static_cast<int>(castlingRights & CastlingRights::Black_O_O)) {
				if ((info.occupied & black_king_side_castling_occ_map) == 0 &&
					(info.attacked & black_king_side_castling_attack_map) == 0)
				{
					append_castling_move<color, state, CastlingType::KingSide>(info);
				}
			}
		} else {
			if (static_cast<int>(castlingRights & CastlingRights::Black_O_O_O)) {
				if ((info.occupied & black_queen_side_castling_occ_map) == 0 &&
					(info.attacked & black_queen_side_castling_attack_map) == 0)
				{
					append_castling_move<color, state, CastlingType::QueenSide>(info);
				}
			}
		}
	}
}

template<Color color, Direction direction, Position::GenerationState state>
inline void Position::generate_slider_move(const PositionInfo& info) const
{
	const PiecesMap& pieces = get_pieces<color>();

	Bitboard fromMap = info.GetNonPin<direction>();

	if constexpr (direction == Direction::Diagonal_1 || direction == Direction::Diagonal_2) {
		fromMap &= (pieces.Bishop() | pieces.Queen());
	} else {
		fromMap &= (pieces.Rook() | pieces.Queen());
	}

	while (fromMap) {
		const Square from = pop_lsb(fromMap);
		Bitboard attackMap = get_slider_attack_map<direction>(from, info.occupied) &
							 info.unoccupied_or_opponent_occupied;

		if constexpr (state == Position::GenerationState::Evasions) {
			attackMap &= info.blockers;
		}

		while (attackMap) {
			const Square to = pop_lsb(attackMap);
			append_move<color, state>(info, from, to);
		}
	}
}

template<Color color, Position::GenerationState state>
inline void Position::generate_slider_diagonal_non_pin_move(const PositionInfo& info) const
{
	const PiecesMap& pieces = get_pieces<color>();
	Bitboard fromMap = pieces.Queen() | pieces.Bishop();

	while (fromMap) {
		const Square from = pop_lsb(fromMap);

		Bitboard attackMap = get_slider_diagonal_attack_map(from, info.occupied) &
							 info.unoccupied_or_opponent_occupied;

		if constexpr (state == Position::GenerationState::Evasions) {
			attackMap &= info.blockers;
		}

		while (attackMap) {
			const Square to = pop_lsb(attackMap);
			append_move<color, state>(info, from, to);
		}
	}
}

template<Color color, Position::GenerationState state>
inline void Position::generate_slider_hv_non_pin_move(const PositionInfo& info) const
{
	const PiecesMap& pieces = get_pieces<color>();
	Bitboard fromMap = pieces.Queen() | pieces.Rook();

	while (fromMap) {
		const Square from = pop_lsb(fromMap);

		Bitboard attackMap =
			get_slider_hv_attack_map(from, info.occupied) & info.unoccupied_or_opponent_occupied;

		if constexpr (state == Position::GenerationState::Evasions) {
			attackMap &= info.blockers;
		}

		while (attackMap) {
			const Square to = pop_lsb(attackMap);
			append_move<color, state>(info, from, to);
		}
	}
}

template<bool UpdateZobrist>
inline void Position::make_move(Move move)
{
	rule50++;

	if (side_to_move() == Color::white) {
		do_make_move<Color::white, UpdateZobrist>(move);
	} else {
		do_make_move<Color::black, UpdateZobrist>(move);
		fullMoveNumber++;
	}

	sideToMove = get_opponent_color(sideToMove);
}

inline void Position::init() { recalc_zobrist_hash(); }

template<Color color, bool UpdateZobrist>
inline void Position::do_make_move(Move move)
{
	if constexpr (UpdateZobrist) {
		constexpr const Color opponent_color = get_opponent_color<color>();
		zobristHash ^= Zobrist::side_to_move_hash(color);
		zobristHash ^= Zobrist::side_to_move_hash(opponent_color);
		if (enPassant) {
			zobristHash ^= Zobrist::en_passant_hash(enPassant);
		}
		zobristHash ^= Zobrist::castling_rights_hash(castlingRights);
	}

	enPassant = 0;

	const Square from = move.from();
	const Square to = move.to();

	switch (move.type()) {
	case MoveType::Normal: do_normal<color, UpdateZobrist>(from, to); break;
	case MoveType::EnPassant: do_en_passant<color, UpdateZobrist>(from, to); break;
	case MoveType::Promotion: do_promotion<color, UpdateZobrist>(move.promotion(), from, to); break;
	case MoveType::Castling: do_castling<color, UpdateZobrist>(move.get_castling_type()); break;
	}

	if constexpr (UpdateZobrist) {
		// new castling rights
		zobristHash ^= Zobrist::castling_rights_hash(castlingRights);
	}
}

template<Color color, bool UpdateZobrist>
inline void Position::do_castling(CastlingType castlingType)
{
	PiecesMap& pieces = get_pieces<color>();

	if constexpr (color == Color::white) {
		castlingRights &= ~CastlingRights::White;

		switch (castlingType) {
		case CastlingType::KingSide: {
			if constexpr (UpdateZobrist) {
				zobristHash ^= Zobrist::piece_hash(color, Square::E1, PieceType::King);
				zobristHash ^= Zobrist::piece_hash(color, Square::G1, PieceType::King);
				zobristHash ^= Zobrist::piece_hash(color, Square::H1, PieceType::Rook);
				zobristHash ^= Zobrist::piece_hash(color, Square::F1, PieceType::Rook);
			}

			pieces.move_piece(PieceType::King, Square::E1, Square::G1);
			pieces.move_piece(PieceType::Rook, Square::H1, Square::F1);
			break;
		}
		case CastlingType::QueenSide: {
			if constexpr (UpdateZobrist) {
				zobristHash ^= Zobrist::piece_hash(color, Square::E1, PieceType::King);
				zobristHash ^= Zobrist::piece_hash(color, Square::C1, PieceType::King);
				zobristHash ^= Zobrist::piece_hash(color, Square::A1, PieceType::Rook);
				zobristHash ^= Zobrist::piece_hash(color, Square::D1, PieceType::Rook);
			}

			pieces.move_piece(PieceType::King, Square::E1, Square::C1);
			pieces.move_piece(PieceType::Rook, Square::A1, Square::D1);
			break;
		}
		}
	} else {
		castlingRights &= ~CastlingRights::Black;

		switch (castlingType) {
		case CastlingType::KingSide: {
			if constexpr (UpdateZobrist) {
				zobristHash ^= Zobrist::piece_hash(color, Square::E8, PieceType::King);
				zobristHash ^= Zobrist::piece_hash(color, Square::G8, PieceType::King);
				zobristHash ^= Zobrist::piece_hash(color, Square::H8, PieceType::Rook);
				zobristHash ^= Zobrist::piece_hash(color, Square::F8, PieceType::Rook);
			}

			pieces.move_piece(PieceType::King, Square::E8, Square::G8);
			pieces.move_piece(PieceType::Rook, Square::H8, Square::F8);
			break;
		}
		case CastlingType::QueenSide: {
			if constexpr (UpdateZobrist) {
				zobristHash ^= Zobrist::piece_hash(color, Square::E8, PieceType::King);
				zobristHash ^= Zobrist::piece_hash(color, Square::C8, PieceType::King);
				zobristHash ^= Zobrist::piece_hash(color, Square::A8, PieceType::Rook);
				zobristHash ^= Zobrist::piece_hash(color, Square::D8, PieceType::Rook);
			}

			pieces.move_piece(PieceType::King, Square::E8, Square::C8);
			pieces.move_piece(PieceType::Rook, Square::A8, Square::D8);
			break;
		}
		}
	}
}

template<Color color, bool UpdateZobrist>
inline void Position::do_promotion(PromotionType promotionType, Square from, Square to)
{
	if constexpr (UpdateZobrist) {
		zobristHash ^= Zobrist::piece_hash(color, from, PieceType::Pawn);
	}

	rule50 = 0;
	update_castling_rights(from, to);

	constexpr const Color opponent_color = get_opponent_color<color>();
	PiecesMap& pieces = get_pieces<color>();
	PiecesMap& enemyPieces = get_pieces<opponent_color>();

	pieces.remove_piece(PieceType::Pawn, from);

	switch (promotionType) {
	case PromotionType::Knight:
		pieces.put_piece(PieceType::Knight, to);
		if constexpr (UpdateZobrist) {
			zobristHash ^= Zobrist::piece_hash(color, to, PieceType::Knight);
		}
		break;
	case PromotionType::Bishop:
		pieces.put_piece(PieceType::Bishop, to);
		if constexpr (UpdateZobrist) {
			zobristHash ^= Zobrist::piece_hash(color, to, PieceType::Bishop);
		}
		break;
	case PromotionType::Rook:
		pieces.put_piece(PieceType::Rook, to);
		if constexpr (UpdateZobrist) {
			zobristHash ^= Zobrist::piece_hash(color, to, PieceType::Rook);
		}
		break;
	case PromotionType::Queen:
		pieces.put_piece(PieceType::Queen, to);
		if constexpr (UpdateZobrist) {
			zobristHash ^= Zobrist::piece_hash(color, to, PieceType::Queen);
		}
		break;
	}

	const PieceType pieceTypeTo = enemyPieces.get_piece_type(to);
	if (pieceTypeTo != PieceType::Empty) {
		// capture
		if constexpr (UpdateZobrist) {
			zobristHash ^= Zobrist::piece_hash(opponent_color, to, pieceTypeTo);
		}
		enemyPieces.remove_piece(pieceTypeTo, to);
	}
}

template<Color color, bool UpdateZobrist>
inline void Position::do_en_passant(Square from, Square to)
{
	constexpr const Color opponent_color = get_opponent_color<color>();
	PiecesMap& pieces = get_pieces<color>();
	PiecesMap& enemyPieces = get_pieces<opponent_color>();

	const Bitboard toBitboard = square_to_bitboard(to);

	rule50 = 0;
	pieces.move_piece(PieceType::Pawn, from, to);

	const Bitboard enemyPawnBitboard = shift<color, Down>(toBitboard);

	enemyPieces.remove_piece(PieceType::Pawn, bitboard_to_square(enemyPawnBitboard));

	if constexpr (UpdateZobrist) {
		const Square ep_next_square = bitboard_to_square(enemyPawnBitboard);
		zobristHash ^= Zobrist::piece_hash(opponent_color, ep_next_square, PieceType::Pawn);
		zobristHash ^= Zobrist::piece_hash(color, from, PieceType::Pawn);
		zobristHash ^= Zobrist::piece_hash(color, to, PieceType::Pawn);
	}
}

template<Color color, bool UpdateZobrist>
inline void Position::do_normal(Square from, Square to)
{
	update_castling_rights(from, to);

	constexpr Color opponent_color = get_opponent_color<color>();
	PiecesMap& pieces      = get_pieces<color>();
	PiecesMap& enemyPieces = get_pieces<opponent_color>();

	const Bitboard fromBB = square_to_bitboard(from);

	const PieceType pieceTypeFrom = pieces.get_piece_type(from);
	const PieceType pieceTypeTo   = enemyPieces.get_piece_type(to);

	pieces.move_piece(pieceTypeFrom, from, to);

	if (pieceTypeFrom == PieceType::Pawn || pieceTypeTo != PieceType::Empty) {
		rule50 = 0;
	}

	if (pieceTypeTo != PieceType::Empty) {
		if constexpr (UpdateZobrist) {
			zobristHash ^= Zobrist::piece_hash(opponent_color, to, pieceTypeTo);
		}
		enemyPieces.remove_piece(pieceTypeTo, to);
	}

	if (pieceTypeFrom == PieceType::Pawn && is_pawn_long_jump<color>(from, to)) {
		enPassant = shift<color, Up>(fromBB);
		if constexpr (UpdateZobrist) {
			zobristHash ^= Zobrist::en_passant_hash(enPassant);
		}
	}

	if constexpr (UpdateZobrist) {
		zobristHash ^= Zobrist::piece_hash(color, from, pieceTypeFrom);
		zobristHash ^= Zobrist::piece_hash(color, to, pieceTypeFrom);
	}
}

inline void Position::recalc_zobrist_hash()
{
	zobristHash = Zobrist::initial_hash();

	if (enPassant) {
		zobristHash ^= Zobrist::en_passant_hash(enPassant);
	}
	zobristHash ^= Zobrist::castling_rights_hash(castlingRights);
	zobristHash ^= Zobrist::side_to_move_hash(side_to_move());

	for (Square square = Square::A1; square <= Square::H8; ++square) {

		const PieceType whitePieceType = get_piece_type<Color::white>(square);
		const PieceType blackPieceType = get_piece_type<Color::black>(square);

		if (whitePieceType != PieceType::Empty) {
			zobristHash ^= Zobrist::piece_hash(Color::white, square, whitePieceType);
		} else if (blackPieceType != PieceType::Empty) {
			zobristHash ^= Zobrist::piece_hash(Color::black, square, blackPieceType);
		}
	}

}

inline constexpr uint8_t castling_rights_mask[64] = {
	13, 15, 15, 15, 12, 15, 15, 14,   // Rank 1:  a1=13, e1=12, h1=14
	15, 15, 15, 15, 15, 15, 15, 15,   // Rank 2
	15, 15, 15, 15, 15, 15, 15, 15,   // Rank 3
	15, 15, 15, 15, 15, 15, 15, 15,   // Rank 4
	15, 15, 15, 15, 15, 15, 15, 15,   // Rank 5
	15, 15, 15, 15, 15, 15, 15, 15,   // Rank 6
	15, 15, 15, 15, 15, 15, 15, 15,   // Rank 7
	 7, 15, 15, 15,  3, 15, 15, 11    // Rank 8:  a8=7, e8=3, h8=11
};

FORCE_INLINE void Position::update_castling_rights(Square from, Square to)
{
	const int m = castling_rights_mask[static_cast<size_t>(from)] &
	              castling_rights_mask[static_cast<size_t>(to)];
	castlingRights &= static_cast<CastlingRights>(m);
}

template<Color color, Position::GenerationState state>
FORCE_INLINE void Position::append_move(const PositionInfo& info, Square from, Square to) const
{
	info.legal_moves->push_back(Move::make_normal(from, to));
}

template<Color color, Position::GenerationState state>
FORCE_INLINE void Position::append_king_move(const PositionInfo& info, Square from, Square to) const
{
	info.legal_moves->push_back(Move::make_normal(from, to));
}

template<Color color, Position::GenerationState state>
FORCE_INLINE void Position::append_promotion_moves(const PositionInfo& info, Square from, Square to) const
{
	info.legal_moves->push_back(Move::make_promotion(from, to, PromotionType::Queen));
	info.legal_moves->push_back(Move::make_promotion(from, to, PromotionType::Knight));
	info.legal_moves->push_back(Move::make_promotion(from, to, PromotionType::Bishop));
	info.legal_moves->push_back(Move::make_promotion(from, to, PromotionType::Rook));
}

template<Color color, Position::GenerationState state>
FORCE_INLINE void Position::append_en_passant_move(const PositionInfo& info, Square from, Square to) const
{
	const PiecesMap& pieces = get_pieces<color>();
    const PiecesMap& opp = get_opponent_pieces<color>();

	const Bitboard captured = get_en_passant_finish_point<color>(enPassant);
	const Bitboard occ_after = (info.occupied & ~square_to_bitboard(from) & ~captured) | square_to_bitboard(to);

	const Square king = bitboard_to_square(pieces.King());
	if (get_slider_hv_attack_map(king, occ_after) & (opp.Rook() | opp.Queen())) {
        return;
    }
    if (get_slider_diagonal_attack_map(king, occ_after) & (opp.Bishop() | opp.Queen())) {
        return;
    }

	info.legal_moves->push_back(Move::make_en_passant(from, to));
}

template<Color color, Position::GenerationState state, CastlingType castlingType>
FORCE_INLINE void Position::append_castling_move(const PositionInfo& info) const
{
	if constexpr (color == Color::white) {
		if constexpr (castlingType == CastlingType::KingSide) {
			info.legal_moves->push_back(Move::make_castling(Square::E1, Square::G1));
		} else {
			info.legal_moves->push_back(Move::make_castling(Square::E1, Square::C1));
		}
	} else {
		if constexpr (castlingType == CastlingType::KingSide) {
			info.legal_moves->push_back(Move::make_castling(Square::E8, Square::G8));
		} else {
			info.legal_moves->push_back(Move::make_castling(Square::E8, Square::C8));
		}
	}
}

inline bool Position::operator==(const Position& other) const
{
	return sideToMove == other.sideToMove && black == other.black && white == other.white &&
		   enPassant == other.enPassant && castlingRights == other.castlingRights;
}

inline std::string move_to_long_notation(Move move)
{
	std::string result = square_to_string(move.from()) + square_to_string(move.to());

	if (move.type() == MoveType::Promotion) {
		switch (move.promotion()) {
		case PromotionType::Knight: result += "n"; break;
		case PromotionType::Bishop: result += "b"; break;
		case PromotionType::Rook:   result += "r"; break;
		case PromotionType::Queen:  result += "q"; break;
		}
	}

	return result;
}

inline Move long_notation_to_move(const std::string& move_str, const Position& pos)
{
	MoveList legal_moves;
	pos.analyze_moves(legal_moves);
	for (Move move : legal_moves) {
		if (move_to_long_notation(move) == move_str) {
			return move;
		}
	}
	assert(false);
	return Move::make_normal(Square::A1, Square::A1);
}

inline Move short_notation_to_move(const std::string& move_str, const Position& pos) 
{
	assert(move_str.size() >= 2);

    if (move_str == "O-O" || move_str == "0-0") {
        if (pos.side_to_move() == Color::white) {
            return Move::make_castling(Square::E1, Square::G1);
        } else {
            return Move::make_castling(Square::E8, Square::G8);
        }
    }
    if (move_str == "O-O-O" || move_str == "0-0-0") {
        if (pos.side_to_move() == Color::white) {
            return Move::make_castling(Square::E1, Square::C1);
        } else {
            return Move::make_castling(Square::E8, Square::C8);
        }
    }

    PieceType piece_type = PieceType::Pawn;
    size_t start_idx = 0;
    switch(move_str[0]) {
        case 'N': piece_type = PieceType::Knight; start_idx = 1; break;
        case 'B': piece_type = PieceType::Bishop; start_idx = 1; break;
        case 'R': piece_type = PieceType::Rook; start_idx = 1; break;
        case 'Q': piece_type = PieceType::Queen; start_idx = 1; break;
        case 'K': piece_type = PieceType::King; start_idx = 1; break;
    }

    std::string to_str;
    if (move_str.back() == '+' || move_str.back() == '#') {
        to_str = move_str.substr(move_str.size() - 3, 2);
    } else {
        to_str = move_str.substr(move_str.size() - 2);
    }
    const Square to = string_to_square(to_str);

    //const bool is_capture = move_str.find('x') != std::string::npos;

    char disamb_file = 0;
    int disamb_rank = -1;
    if (start_idx < move_str.size() - 2) {
        for (size_t i = start_idx; i < move_str.size() - 2; ++i) {
            if (move_str[i] >= 'a' && move_str[i] <= 'h' && move_str[i] != 'x') {
                disamb_file = move_str[i];
            }
            if (move_str[i] >= '1' && move_str[i] <= '8') {
                disamb_rank = move_str[i] - '1';
            }
        }
    }

    Square from = Square::A1;
    MoveList moves;
    pos.analyze_moves(moves);

    for (const Move& move : moves) {
        if (move.to() != to) continue;
        
        Square candidate_from = move.from();
        if (pos.get_piece_type(candidate_from) != piece_type) {
			continue;
		}
        
        if (disamb_file && get_file(candidate_from) != (disamb_file - 'a')) {
			continue;
		}
        if (disamb_rank != -1 && get_rank(candidate_from) != disamb_rank)  {
			continue;
		}

        from = candidate_from;
        break;
    }

    PromotionType promotion = PromotionType::Queen;
    size_t prom_index = move_str.find('=');
    if (prom_index != std::string::npos && prom_index + 1 < move_str.size()) {
        switch (move_str[prom_index + 1]) {
            case 'Q': promotion = PromotionType::Queen; break;
            case 'R': promotion = PromotionType::Rook; break;
            case 'B': promotion = PromotionType::Bishop; break;
            case 'N': promotion = PromotionType::Knight; break;
        }
        return Move::make_promotion(from, to, promotion);
    }

    return Move::make_normal(from, to);
}

} // namespace deepchess
