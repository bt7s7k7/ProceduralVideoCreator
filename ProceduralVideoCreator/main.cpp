#include "pch.h"
#include "constants.h"

std::filesystem::path filePath;
auto options = cxxopts::Options(PROGRAM_NAME, "{}\n");

bool setup(const cxxopts::ParseResult& result);
bool update();

int main(int argc, char** args) {
	spdlog::info("Starting {}...", PROGRAM_NAME);
	spdlog::info("by Branislav Trstensk`y");

	
	options.add_options()
		(FILE_ARG_DEFINITION, FILE_ARG_DESC, cxxopts::value<std::string>())
		;

	auto parsed = options.parse(argc, args);

	if (!setup(parsed)) return 1;

	while (update()) {}

}