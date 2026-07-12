#pragma once
#include <cstdint>
#include <string>

namespace tortoise {

enum Direction : int {
	Left = -1,
	Right = 1,
	Up = 8,
	Down = -8,
	UpLeft = 7,
	UpRight = 9,
	DownLeft = -9,
	DownRight = -7,

	Horizontal = Left,
	Vertical = Up,
	Diagonal_1 = UpLeft,
	Diagonal_2 = UpRight
};

template<Direction direction>
inline constexpr Direction opposite_direction()
{
	if constexpr (direction == Left) {
		return Right;
	}
	if constexpr (direction == Right) {
		return Left;
	}
	if constexpr (direction == Up) {
		return Down;
	}
	if constexpr (direction == Down) {
		return Up;
	}
	if constexpr (direction == UpLeft) {
		return DownRight;
	}
	if constexpr (direction == UpRight) {
		return DownLeft;
	}
	if constexpr (direction == DownLeft) {
		return UpRight;
	}
	if constexpr (direction == DownRight) {
		return UpLeft;
	}
	return Horizontal;
}

enum File : int {
	FileA,
	FileB,
	FileC,
	FileD,
	FileE,
	FileF,
	FileG,
	FileH,

	FileCount
};

inline File& operator--(File& file)
{
	file = static_cast<File>(static_cast<int>(file) - 1);
	return file;
}

inline File& operator++(File& file)
{
	file = static_cast<File>(static_cast<int>(file) + 1);
	return file;
}

inline std::string file_to_string(File file)
{
	std::string result;
	result += static_cast<char>('a' + static_cast<int>(file));
	return result;
}

enum Rank : int {
	Rank1,
	Rank2,
	Rank3,
	Rank4,
	Rank5,
	Rank6,
	Rank7,
	Rank8,

	RankCount
};

inline Rank& operator--(Rank& rank)
{
	rank = static_cast<Rank>(static_cast<int>(rank) - 1);
	return rank;
}

inline Rank& operator++(Rank& rank)
{
	rank = static_cast<Rank>(static_cast<int>(rank) + 1);
	return rank;
}

enum class Color {
	white = 0,
	black = 1
};
constexpr int color_count = 2;

inline Color get_opponent_color(Color color)
{
	return color == Color::white ? Color::black : Color::white;
}

template<Color color>
inline constexpr Color get_opponent_color()
{
	if constexpr (color == Color::white) {
		return Color::black;
	} else {
		return Color::white;
	}
}

enum class Square : int {
	SquareNone = -1,
	A1, B1, C1, D1, E1, F1, G1, H1, // 0-7
	A2, B2, C2, D2, E2, F2, G2, H2, // 8-25
	A3, B3, C3, D3, E3, F3, G3, H3, // 16-23
	A4, B4, C4, D4, E4, F4, G4, H4, // 24-31
	A5, B5, C5, D5, E5, F5, G5, H5, // 32-39
	A6, B6, C6, D6, E6, F6, G6, H6, // 40-47
	A7, B7, C7, D7, E7, F7, G7, H7, // 48-55
	A8, B8, C8, D8, E8, F8, G8, H8 // 56-63
};

const int square_count = 64;

inline Square operator+(Square square, int shift)
{
	return static_cast<Square>(static_cast<int>(square) + shift);
}

inline Square operator-(Square square, int shift)
{
	return static_cast<Square>(static_cast<int>(square) - shift);
}

inline Square& operator++(Square& square)
{
	square = static_cast<Square>(static_cast<int>(square) + 1);
	return square;
}

inline std::string square_to_string(Square square)
{
	static constexpr const char* const square_names[65] = {
		"-", 
		"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
		"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
		"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
		"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
		"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
		"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
		"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
		"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
	};

	const int index = static_cast<int>(square) + 1;
	return square_names[index];
}

inline Square string_to_square(const std::string& str)
{
	const int file = str[0] - 'a';
	const int rank = str[1] - '1';

	return static_cast<Square>(rank * 8 + file);
}

inline File get_file(Square square)
{
	return static_cast<File>(static_cast<int>(square) % 8);
}

inline Rank get_rank(Square square)
{
	return static_cast<Rank>(static_cast<int>(square) / 8);
}

inline Square MakeSquare(Rank rank, File file)
{
	return static_cast<Square>(static_cast<int>(file) + static_cast<int>(rank) * 8);
}


enum class CastlingRights {
	None = 0,

	White_O_O = 1,
	White_O_O_O = 2,

	Black_O_O = 4,
	Black_O_O_O = 8,

	Black = Black_O_O | Black_O_O_O,
	White = White_O_O | White_O_O_O,

	KingSide = Black_O_O | White_O_O,
	QueenSide = Black_O_O_O | White_O_O_O,

	All = Black | White
};
const int CastlingRightsCount = 16;

inline CastlingRights operator&(CastlingRights first, CastlingRights second)
{
	return static_cast<CastlingRights>(static_cast<int>(first) & static_cast<int>(second));
}

inline CastlingRights& operator&=(CastlingRights& first, CastlingRights second)
{
	first = static_cast<CastlingRights>(static_cast<int>(first) & static_cast<int>(second));
	return first;
}

inline CastlingRights& operator|=(CastlingRights& first, CastlingRights second)
{
	first = static_cast<CastlingRights>(static_cast<int>(first) | static_cast<int>(second));
	return first;
}

inline CastlingRights operator~(CastlingRights other)
{
	return static_cast<CastlingRights>(~static_cast<uint32_t>(other));
}


enum class PieceType : int {
	Empty,
	Pawn,
	Knight,
	Bishop,
	Rook,
	Queen,
	King
};
constexpr int PieceTypeCount = 7;

enum class Piece : int {
	Empty,

	BlackPawn,
	BlackKnight,
	BlackBishop,
	BlackRook,
	BlackQueen,
	BlackKing,
	
	WhitePawn,
	WhiteKnight,
	WhiteBishop,
	WhiteRook,
	WhiteQueen,
	WhiteKing,
};

inline PieceType get_piece_type(Piece piece)
{
	switch (piece) {
	case Piece::Empty:
		return PieceType::Empty;
	case Piece::BlackPawn:
	case Piece::WhitePawn:
		return PieceType::Pawn;
	case Piece::BlackKnight:
	case Piece::WhiteKnight:
		return PieceType::Knight;
	case Piece::BlackBishop:
	case Piece::WhiteBishop:
		return PieceType::Bishop;

	case Piece::BlackRook:
	case Piece::WhiteRook:
		return PieceType::Rook;
	case Piece::BlackQueen:
	case Piece::WhiteQueen:
		return PieceType::Queen;
	case Piece::BlackKing:
	case Piece::WhiteKing:
		return PieceType::King;
	}
	return PieceType::Empty;
}

inline constexpr Color get_piece_color(Piece piece)
{
	switch (piece) {
	case Piece::Empty:
	case Piece::BlackPawn:
	case Piece::BlackKnight:
	case Piece::BlackBishop:
	case Piece::BlackRook:
	case Piece::BlackQueen:
	case Piece::BlackKing:
		return Color::black;
	case Piece::WhitePawn:
	case Piece::WhiteKnight:
	case Piece::WhiteBishop:
	case Piece::WhiteRook:
	case Piece::WhiteQueen:
	case Piece::WhiteKing:
		return Color::white;
	}
	return Color::white;
}

inline constexpr Piece make_piece(PieceType type, Color color)
{
	if (type == PieceType::Empty) {
		return Piece::Empty;
	}

	return static_cast<Piece>(static_cast<int>(type)  + ((1-static_cast<int>(color))*6));
}

inline std::string piece_type_to_string(PieceType type)
{
	switch (type) {
	case PieceType::Pawn:
		return "";
	case PieceType::Knight:
		return "N";
	case PieceType::Bishop:
		return "B";
	case PieceType::Queen:
		return "Q";
	case PieceType::King:
		return "K";
	case PieceType::Rook:
		return "R";
	default:
		return "";
	}
}

} // namespace tortoise