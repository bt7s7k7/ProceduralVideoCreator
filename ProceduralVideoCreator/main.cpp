#include "pch.h"
#include "constants.h"
#include "SDLHelper.h"

auto options = cxxopts::Options(PROGRAM_NAME, "{}\n");

bool setup(const cxxopts::ParseResult& result, const cxxopts::Options& options);
bool updateLoop();

int main(int argc, char** args) try {
	spdlog::info("Starting {}...", PROGRAM_NAME);
	spdlog::info("by Branislav Trstensk`y");

	
	options.add_options()
		(FILE_ARG_DEFINITION, FILE_ARG_DESC, cxxopts::value<std::string>())
		(NOHOTSWAP_ARG_DEFINITION, NOHOTSWAP_ARG_DESC)
		(DEBUG_ARG_DEFINITION, DEBUG_ARG_DESC)
		(SCALE_ARG_DEFINITION, SCALE_ARG_DESC, cxxopts::value<int>()->default_value("2"))
		;

	auto parsed = options.parse(argc, args);

	spdlog::info("Initializing SDL...");
	handleSDLError(SDL_Init(SDL_INIT_VIDEO));
	handleSDLError(TTF_Init());

	std::atexit([]() {
		SDL_Quit();
	});

	if (!setup(parsed, options)) return 1;

	if (!updateLoop()) return 1;

} catch (const std::exception& err) {
	// Catching all errors and putting them to the log
	spdlog::critical("[Abort] " + std::string(err.what()));
	throw; // Rethrow the error for debugger
}