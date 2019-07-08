#include "pch.h"
#include "constants.h"
#include "globals.h"

void createLuaBindings();

bool setup(const cxxopts::ParseResult& result, const cxxopts::Options & options) {
	// Test if arg --file is present
	if (result.count(FILE_ARG_NAME) != 1) {
		spdlog::error(options.help().c_str(), "Argument --file is missing or duplicated");
		return false;
	}

	// Put arg values into globals
	filePath = result[FILE_ARG_NAME].as<std::string>();
	ignoreChanges = result.count(NOHOTSWAP_ARG_NAME) > 0 || result[NOHOTSWAP_ARG_NAME].as<bool>();
	maxPreviewSize = result[MAXS_ARG_NAME].as<int>();

	// Set the desired log level
	if (result.count(DEBUG_ARG_NAME) > 0 || result[DEBUG_ARG_NAME].as<bool>()) {
		spdlog::info("Setting debug level to debug as per --debug");
		spdlog::set_level(spdlog::level::debug);
	}

	// Test the provided file
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

	spdlog::info("Creating Lua bindings...");
	createLuaBindings();

	return true;
}