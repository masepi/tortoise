
#include <filesystem>
#include <string>
#include <fstream>
#include <vector>
#include <cassert>


#include "epd.h"
#include "rules.h"
#include "engine.h"

namespace fs = std::filesystem;

std::vector<PerftTestCase> parse_perft_test_file(const std::filesystem::path& path)
{
	std::vector<PerftTestCase> test_cases;

	std::ifstream wif(path);

	while (!wif.eof()) {
		std::string test_case_line;
		std::getline(wif, test_case_line);

		if (test_case_line.empty()) {
			return test_cases;
		}

		size_t begin = 0;
		size_t end = test_case_line.find(';', begin);
		if (end == std::string::npos) {
			end = test_case_line.size();
		}

		const std::string fen_str = test_case_line.substr(begin, end-begin);

		do {
			begin = end;
			begin++;
			end = test_case_line.find(';', begin);
			if (end==std::string::npos) {
				end = test_case_line.size();
			}

			const std::string test_case_str = test_case_line.substr(begin, end-begin);

			const size_t dPos = test_case_str.find('D');
			const size_t space_pos = test_case_str.find(' ', dPos);
			assert(space_pos != std::string::npos);
			
			const std::string depthStr = test_case_str.substr(dPos + 1, space_pos - dPos - 1);
			const int depth = std::stoi(depthStr);

			const std::string count_str = test_case_str.substr(space_pos+1);
			const uint64_t count = std::stoull(count_str);

			PerftTestCase test_case(fen_str, depth, count);
			test_cases.push_back(test_case);
		} while (end != test_case_line.size());
	}
	return test_cases;
}

std::vector<PerftTestCase> parse_perft_test_folder(const std::filesystem::path& path)
{
	assert(fs::exists(path) && fs::is_directory(path));

	std::vector<std::vector<PerftTestCase>> all_test_cases;

	for (const auto& entry : fs::recursive_directory_iterator(path)) {
		if (!fs::is_regular_file(entry.status())) {
			continue;
		}

		auto test_cases = parse_perft_test_file(entry.path());
		all_test_cases.push_back(test_cases);
	}

	size_t total_count = 0;
	for (const auto& item: all_test_cases) {
		total_count += item.size();
	}

	std::vector<PerftTestCase> result;
	result.reserve(total_count);
	for (const auto& item: all_test_cases) {
		result.insert(result.end(), item.begin(), item.end());
	}
	return result;
}
