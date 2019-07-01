#include "pch.h"
#include "constants.h"
#include "globals.h"

void createLuaBindings();

bool setup(const cxxopts::ParseResult& result, const cxxopts::Options & options) {
	if (result.count(FILE_ARG_NAME) != 1) {
		spdlog::error(options.help().c_str(), "Argument --file is missing");
		return false;
	}

	filePath = result[FILE_ARG_NAME].as<std::string>();
	ignoreChanges = result.count(NOHOTSWAP_ARG_NAME) > 0 || result[NOHOTSWAP_ARG_NAME].as<bool>();
	if (result.count(DEBUG_ARG_NAME) > 0 || result[DEBUG_ARG_NAME].as<bool>()) {
		spdlog::info("Setting debug level to debug as per --debug");
		spdlog::set_level(spdlog::level::debug);
	}
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

	spdlog::info("Creating SDL windows...");

	sliderWindow.reset(handleSDLError(SDL_CreateWindow("Slider", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 700, 300, SDL_WINDOW_RESIZABLE)));
	previewWindow.reset(handleSDLError(SDL_CreateWindow("Preview", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 700, 700, SDL_WINDOW_RESIZABLE)));

	sliderWindowRenderer.reset(handleSDLError(SDL_CreateRenderer(&*sliderWindow, -1, SDL_RENDERER_ACCELERATED)));



	return true;
}