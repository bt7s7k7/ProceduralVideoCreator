#include "pch.h"
#include "constants.h"

extern std::filesystem::path filePath;
extern cxxopts::Options options;

void loadLua();
void createLuaBindings();

bool setup(const cxxopts::ParseResult& result) {
	if (result.count(FILE_ARG_NAME) != 1) {
		spdlog::error(options.help().c_str(), "Argument --file is missing");
		return false;
	}

	filePath = result[FILE_ARG_NAME].as<std::string>();
	try {
		if (!std::filesystem::is_regular_file(filePath)) {
			spdlog::error("Specified file is not a file");
			return false;
		}
	} catch (const std::filesystem::filesystem_error&) {
		spdlog::error("Specified file doesn't exist");
		return false;
	}
	if (filePath.extension() != ".lua") {
		spdlog::error("Specified lua file is not a lua file");
		return false;
	}

	createLuaBindings();

	loadLua();
	return true;
}