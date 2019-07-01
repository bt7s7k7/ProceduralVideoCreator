#include "pch.h"
#include "constants.h"

auto options = cxxopts::Options(PROGRAM_NAME, "{}\n");

bool setup(const cxxopts::ParseResult& result, const cxxopts::Options& options);
bool update();

int main(int argc, char** args) {
	spdlog::info("Starting {}...", PROGRAM_NAME);
	spdlog::info("by Branislav Trstensk`y");

	
	options.add_options()
		(FILE_ARG_DEFINITION, FILE_ARG_DESC, cxxopts::value<std::string>()),
		(NOHOTSWAP_ARG_DEFINITION, NOHOTSWAP_ARG_DESC)
		;

	auto parsed = options.parse(argc, args);

	if (!setup(parsed, options)) return 1;

	

}