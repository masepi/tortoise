#include "fen.h"

#include <limits>
#include <cassert>

namespace tortoise {
namespace {

// This class is responsible for parsing FEN and creating chess position.
class FenReader {
public:
	explicit FenReader(std::string_view _fen) : fen(_fen), pos(0) {}

	bool load(Position& p);
	bool load_epd(Fen::EpdTest& test);

private:
	std::string_view fen;
	size_t pos;

	bool read_rank(Position& p, Rank rank);
	bool read_piece(Position& p, Rank rank, File file);
	bool read_side_to_move(Position& p);
	bool read_castling_rights(Position& p);
	bool read_en_passant(Position& p);
	bool read_rule50(Position& p);
	bool read_full_move_number(Position& p);
	bool read_int(int& number);
	bool read_epd_best_move(std::string& move);
};

bool validate_position(const Position& p)
{
	const PiecesMap& white = p.get_map<Color::white>();
	const PiecesMap& black = p.get_map<Color::black>();
	if (!std::has_single_bit(white.King()) || !std::has_single_bit(black.King())) {
		return false;
	}

	if ((white.Pawn() | black.Pawn()) & (rank_1_map | rank_8_map)) {
		return false;
	}

	const Square white_king = bitboard_to_square(white.King());
	if (get_king_attack_map(white_king) & black.King()) {
		return false;
	}

	return true;
}

bool FenReader::load(Position& p)
{
	for (Rank rank = Rank::Rank8; rank >= Rank::Rank1; --rank ) {
		if (!read_rank(p, rank)) {
			return false;
		}
		if (rank != Rank::Rank1) {
			pos++;
		}
	}

	if (!read_side_to_move(p)) {
		return false;
	}

	if (!read_castling_rights(p)) {
		return false;
	}

	if (!read_en_passant(p)) {
		return false;
	}

	if (!read_rule50(p)) {
		return false;
	}

	if (!read_full_move_number(p)) {
		return false;
	}
	while (pos < fen.length() && fen[pos] == ' ') {
		pos++;
	}
	if (pos != fen.length() || !validate_position(p)) {
		return false;
	}
	pos = 0;
	return true;
}

bool FenReader::load_epd(Fen::EpdTest& test)
{
	Position& p = test.position;

	for (Rank rank = Rank::Rank8; rank >= Rank::Rank1; --rank ) {
		if (!read_rank(p, rank)) {
			return false;
		}
		if (rank != Rank::Rank1) {
			pos++;
		}
	}

	if (!read_side_to_move(p)) {
		return false;
	}

	if (!read_castling_rights(p)) {
		return false;
	}

	if (!read_en_passant(p)) {
		return false;
	}

	if (!read_epd_best_move(test.best_move)) {
		return false;
	}
	if (!validate_position(p)) {
		return false;
	}
	test.description = fen.substr(pos + 2, fen.size() - pos - 2);

	// reset pos.
	pos = 0;

	return true;
}

bool FenReader::read_rank(Position& p, Rank rank)
{
	File file = File::FileA;
	int squares_read = 0;
	while (pos < fen.length() && fen[pos] != ' ' && fen[pos] != '/') {
		const char c = fen[pos];

		const Square square = MakeSquare(rank, file);

		switch (c) {
		case 'r':
			p.set_piece(square, Piece::BlackRook);
			break;
		case 'b':
			p.set_piece(square, Piece::BlackBishop);
			break;
		case 'q':
			p.set_piece(square, Piece::BlackQueen);
			break;
		case 'p':
			p.set_piece(square, Piece::BlackPawn);
			break;
		case 'n':
			p.set_piece(square, Piece::BlackKnight);
			break;
		case 'k':
			p.set_piece(square, Piece::BlackKing);
			break;
		case 'R':
			p.set_piece(square, Piece::WhiteRook);
			break;
		case 'B':
			p.set_piece(square, Piece::WhiteBishop);
			break;
		case 'Q':
			p.set_piece(square, Piece::WhiteQueen);
			break;
		case 'P':
			p.set_piece(square, Piece::WhitePawn);
			break;
		case 'N':
			p.set_piece(square, Piece::WhiteKnight);
			break;
		case 'K':
			p.set_piece(square, Piece::WhiteKing);
			break;

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		{
			const int count = c - '0';
			if (count <= 0 || count > 8 || squares_read + count > 8) {
				return false;
			}

			for (int i = 0; i < count - 1; i++) {
				const Square empty_square = MakeSquare(rank, file);
				p.set_piece(empty_square, Piece::Empty);
				++file;
			}
			const Square empty_square = MakeSquare(rank, file);
			p.set_piece(empty_square, Piece::Empty);
			squares_read += count - 1;
			break;
		}
		default:
			return false;
		}

		pos++;
		squares_read++;
		if (file != File::FileH) {
			++file;
		} else {
			break;
		}
	}

	if (pos >= fen.length() || squares_read != 8) {
		return false;
	}
	const char c = fen[pos];
	if ( (rank > Rank::Rank1 && c != '/') || (rank == Rank::Rank1 && c != ' ')) {
		return false;
	}
	return true;
}


bool FenReader::read_side_to_move(Position& p)
{
	if (pos + 2 >= fen.length()) {
		return false;
	}

	pos++;

	const char c = fen[pos];
	switch (c) {
	case 'w':
		p.set_side_to_move(Color::white);
		break;
	case 'b':
		p.set_side_to_move(Color::black);
		break;
	default:
		return false;
	}
	pos++;

	if (fen[pos] != ' ') {
		return false;
	}
	return true;
}

bool FenReader::read_castling_rights(Position& p)
{
	if (pos + 2 >= fen.length()) {
		return false;
	}

	pos++;


	CastlingRights castlingRights = CastlingRights::None;
	while(pos < fen.length() && fen[pos] != ' ') {
		const char c = fen[pos];
		switch (c) {
		case '-':
			if (pos + 1 >= fen.length() || fen[pos + 1] != ' ' || castlingRights != CastlingRights::None) {
				return false;
			}
			break;
		case 'K':
			castlingRights |= CastlingRights::White_O_O;
			break;
		case 'Q':
			castlingRights |= CastlingRights::White_O_O_O;
			break;
		case 'k':
			castlingRights |= CastlingRights::Black_O_O;
			break;
		case 'q':
			castlingRights |= CastlingRights::Black_O_O_O;
			break;
		default:
			return false;
		}
		pos++;
	}

	p.set_castling_rights(castlingRights);

	if (fen[pos] != ' ') {
		return false;
	}
	return true;
}

bool FenReader::read_en_passant(Position& p)
{
	if (pos + 3 >= fen.length()) {
		return false;
	}
	pos++;

	const char first = fen[pos];
	pos++;

	if (first == '-') {
		p.set_en_passant(Square::SquareNone);
		if (fen[pos] != ' ') {
			return false;
		}
		return true;
	}

	if (!('a' <= first && first <= 'h')) {
		return false;
	}

	const char second = fen[pos];
	if (second != '3' && second != '6') {
		return false;
	}
	if ((p.side_to_move() == Color::white && second != '6') ||
		(p.side_to_move() == Color::black && second != '3')) {
		return false;
	}

	pos++;

	if (fen[pos] != ' ') {
		return false;
	}

	const Square en_passant = static_cast<Square>(8 * (second - '1') + (first - 'a'));
	p.set_en_passant(en_passant);
	
	return true;
}

bool FenReader::read_rule50(Position& p)
{
	pos++;

	if (pos >= fen.length()) {
		return false;
	}

	int rule50 = 0;
	if (!read_int(rule50)) {
		return false;
	}
	p.set_rule50(rule50);

	return true;
}

bool FenReader::read_full_move_number(Position& p)
{
	pos++;

	if (pos >= fen.length()) {
		return false;
	}

	int moves_count = 0;
	if (!read_int(moves_count)) {
		return false;
	}
	if (moves_count < 1) {
		return false;
	}
	p.set_full_move_number(moves_count);

	return true;
}

bool FenReader::read_int(int& number)
{
	if (pos >= fen.length()) {
		return false;
	}

	number = 0;
	int count = 0;
	while (pos < fen.length() && fen[pos] != ' ') {
		const char c = fen[pos];
		if (!('0' <= c && c <= '9')) {
			return false;
		}
		const int digit = c - '0';
		if (number > (std::numeric_limits<int>::max() - digit) / 10) {
			return false;
		}
		number = number * 10 + digit;
		pos++;
		count++;
	}

	return count > 0;
}

bool FenReader::read_epd_best_move(std::string& move)
{
	if (pos + 7 >= fen.size()) {
		return false;
	}
	if (fen.substr(pos, 4) != " bm ") {
		return false;
	}
	pos += 4;

	const size_t start_pos = pos;
	while (pos < fen.size() && fen[pos] != ';') {
		pos++;
	}
	move = fen.substr(start_pos, pos - start_pos);
	return true;
}

} // namespace 

namespace Fen {

const std::string initial_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

std::string save(const Position& p)
{
	std::string fen;
	for (Rank rank = Rank::Rank8; rank >= Rank::Rank1; --rank) {
		int empty_count = 0;
		for (File file = File::FileA; file <= File::FileH; ++file) {
			const Square square = MakeSquare(rank, file);
			const Piece piece = p.get_piece(square);

			if (piece != Piece::Empty && empty_count != 0) {
				fen += std::to_string(empty_count);
				empty_count = 0;
			}

			switch (piece) {
			case Piece::Empty:
				empty_count++;
				break;
			case Piece::BlackPawn:
				fen += "p";
				break;
			case Piece::WhitePawn:
				fen += "P";
				break;
			case Piece::BlackKnight:
				fen += "n";
				break;
			case Piece::WhiteKnight:
				fen += "N";
				break;
			case Piece::BlackBishop:
				fen += "b";
				break;
			case Piece::WhiteBishop:
				fen += "B";
				break;
			case Piece::BlackRook:
				fen += "r";
				break;
			case Piece::WhiteRook:
				fen += "R";
				break;
			case Piece::BlackQueen:
				fen += "q";
				break;
			case Piece::WhiteQueen:
				fen += "Q";
				break;
			case Piece::BlackKing:
				fen += "k";
				break;
			case Piece::WhiteKing:
				fen += "K";
				break;
			}
		}
		if (empty_count != 0) {
			fen += std::to_string(empty_count);
		}

		if (rank != Rank::Rank1) {
			fen += "/";
		}
	}

	fen += " ";
	if (p.side_to_move() == Color::white) {
		fen += "w ";
	} else {
		fen += "b ";
	}

	const CastlingRights castlingRights = p.get_castling_rights();

	if (castlingRights == CastlingRights::None) {
		fen += "- ";
	} else {
		if ((castlingRights & CastlingRights::White_O_O) != CastlingRights::None ) {
			fen += "K";
		} 
		if ((castlingRights & CastlingRights::White_O_O_O) != CastlingRights::None) {
			fen += "Q";
		} 
		if ((castlingRights & CastlingRights::Black_O_O) != CastlingRights::None) {
			fen += "k";
		} 
		if ((castlingRights & CastlingRights::Black_O_O_O) != CastlingRights::None) {
			fen += "q";
		}
		fen += " ";
	}

	const Square en_passant = p.get_en_passant();
	if (en_passant == Square::SquareNone) {
		fen += "- ";
	} else {
		fen += square_to_string(en_passant);
		fen += " ";
	}

	fen += std::to_string(p.get_rule50());
	fen += " ";
	fen += std::to_string(p.get_full_move_number());
	return fen;
}

bool load( const std::string& fen, Position& position)
{
	FenReader reader{ fen };
	Position parsed;
	if (!reader.load(parsed)) {
		return false;
	}
	parsed.init();
	position = std::move(parsed);
	return true;
}

Position load(const std::string_view fen)
{
	FenReader reader{ fen };
	Position position;
	[[maybe_unused]] const bool success = reader.load(position);
	assert(success);
	position.init();
	return position;
}

EpdTest load_epd_test(std::string_view epd)
{
	FenReader reader{ epd };
	EpdTest test;
	const bool success = reader.load_epd(test);
	assert(success);

	test.position.init();
	return test;
}


} // namespace Fen
} // namespace tortoise
