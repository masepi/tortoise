#include <string>
#include <cstdint>
#include <vector>
#include <filesystem>


struct PerftTestCase {
	std::string fen;
	int depth;
	uint64_t count;

	PerftTestCase() : depth(0), count(0) {}
	PerftTestCase(std::string_view _fen, int _depth, uint64_t _count) : 
		fen(_fen), depth(_depth), count(_count) {}
};


std::vector<PerftTestCase> parse_perft_test_file(const std::filesystem::path& path);
std::vector<PerftTestCase> parse_perft_test_folder(const std::filesystem::path& folder);

