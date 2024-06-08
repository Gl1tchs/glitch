#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

struct BundleFileData {
	const char* path;
	size_t start_idx;
	size_t size;
};

#ifdef _WIN32
static bool get_file_size(const std::string& file_path, size_t& file_size) {
	WIN32_FILE_ATTRIBUTE_DATA file_attrs;
	if (!GetFileAttributesExA(
				file_path.c_str(), GetFileExInfoStandard, &file_attrs)) {
		return false;
	}
	file_size = static_cast<size_t>(file_attrs.nFileSizeLow);
	return true;
}
#else
static bool get_file_size(const std::string& file_path, size_t& file_size) {
	struct stat st {};
	if (stat(file_path.c_str(), &st) != 0) {
		return false;
	}
	file_size = static_cast<size_t>(st.st_size);
	return true;
}
#endif

static void bundle(const std::string& file_path,
		const std::vector<std::string>& input_files) {
	const std::string file_name =
			file_path.substr(0, file_path.find_last_of('.'));

	std::ofstream file(file_path);
	if (!file.is_open()) {
		std::cerr << "Error: Unable to open file " << file_path << std::endl;
		return;
	}

	file << "#pragma once\n\n";

	file << "#include <cstdint>\n";
	file << "#include <cstddef>\n\n";

	file << "struct BundleFileData {\n";
	file << "\tconst char* path;\n";
	file << "\tsize_t start_idx;\n";
	file << "\tsize_t size;\n";
	file << "};\n\n";

	file << "inline size_t BUNDLE_FILE_COUNT = " << input_files.size() << ";\n";
	file << "inline BundleFileData BUNDLE_FILES[] = {\n";

	size_t total_size = 0;
	for (size_t idx = 0; idx < input_files.size(); ++idx) {
		size_t size;
		if (!get_file_size(input_files[idx], size)) {
			std::cerr << "Error: Unable to get file size for "
					  << input_files[idx] << std::endl;
			return;
		}

		file << "\t" << std::filesystem::path(input_files[idx]).filename()
			 << ", " << total_size << ", " << size << ",\n";
		total_size += size;
	}
	file << "};\n\n";

	uint8_t hex_counter = 0;

	file << "inline uint8_t BUNDLE_DATA[] = {";
	for (const std::string& current_file : input_files) {
		std::ifstream f(current_file, std::ios::binary);
		if (!f.is_open()) {
			std::cerr << "Error: Unable to open file " << current_file
					  << std::endl;
			return;
		}

		file << "\n/* " << current_file << " */\n\t";

		uint8_t byte;
		while (f.get(reinterpret_cast<char&>(byte))) {
			file << "0x" << std::hex << std::uppercase << std::setw(2)
				 << std::setfill('0') << static_cast<unsigned>(byte) << ", ";
			// new line if 20 bytes written
			if (++hex_counter >= 12) {
				file << "\n\t";
				hex_counter = 0;
			}
		}
	}

	file << "\n};\n\n";

	file.close();
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0]
				  << " <output_file> <input_file1> [<input_file2> ...]\n";
		return 1;
	}

	std::string output_file = argv[1];
	std::vector<std::string> input_files;
	for (int i = 2; i < argc; ++i) {
		input_files.push_back(argv[i]);
	}

	std::ofstream output(output_file);
	if (!output.is_open()) {
		std::cerr << "Error: Unable to open output file " << output_file
				  << std::endl;
		return 1;
	}

	bundle(output_file, input_files);

	return 0;
}
