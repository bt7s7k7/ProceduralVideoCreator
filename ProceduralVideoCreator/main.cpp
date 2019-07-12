#include "pch.h"
#include "constants.h"
#include "SDLHelper.h"
#include "globals.h"

auto options = cxxopts::Options(PROGRAM_NAME, "{}\n");

bool setup(const cxxopts::ParseResult& result, const cxxopts::Options& options);
bool updateLoop();

int main(int argc, char** args) try {
	spdlog::info("Starting {}...", PROGRAM_NAME);
	spdlog::info("by Branislav Trstensk`y");

	exePath = args[0];

	
	options.add_options()
		(FILE_ARG_DEFINITION, FILE_ARG_DESC, cxxopts::value<std::string>())
		(NOHOTSWAP_ARG_DEFINITION, NOHOTSWAP_ARG_DESC)
		(DEBUG_ARG_DEFINITION, DEBUG_ARG_DESC)
		(MAXS_ARG_DEFINITION, MAXS_ARG_DESC, cxxopts::value<int>()->default_value("960"))
		;

	auto parsed = options.parse(argc, args);

	spdlog::info("Initializing SDL...");
	handleSDLError(SDL_Init(SDL_INIT_VIDEO));
	handleSDLError(TTF_Init());
	handleSDLError(IMG_Init(0));

	std::atexit([]() {
		SDL_Quit();
		IMG_Quit();
		TTF_Quit();
	});

	if (!setup(parsed, options)) return 1;

	spdlog::debug("Executable path: {}", exePath.string());

	if (!updateLoop()) return 1;

} catch (const std::exception& err) {
	// Catching all errors and putting them to the log
	spdlog::critical("[Abort] " + std::string(err.what()) + "\n// For anyone debugging, this error was logged from main catch");
	throw; // Rethrow the error for debugger
}