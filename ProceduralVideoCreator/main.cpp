#include "pch.h"
#include "constants.h"

kaguya::State luaState;
std::filesystem::path filePath;
std::filesystem::file_time_type fileLastModified;
auto options = cxxopts::Options(PROGRAM_NAME, "{}\n");

void loadLua() {
	spdlog::info("Loading script...");
	fileLastModified = std::filesystem::last_write_time(filePath);
	luaState.dofile(filePath.string());
	spdlog::info("Loaded!");
}

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
	} catch (const std::filesystem::filesystem_error &) {
		spdlog::error("Specified file doesn't exist");
		return false;
	}
	if (filePath.extension() != ".lua") {
		spdlog::error("Specified lua file is not a lua file");
		return false;
	}

	

	loadLua();
	return true;
}
void update() {

}

int main(int argc, char** args) {
	spdlog::info("Starting {}...", PROGRAM_NAME);
	spdlog::info("by Branislav Trstensk`y");

	
	options.add_options()
		(FILE_ARG_DEFINITION, FILE_ARG_DESC, cxxopts::value<std::string>())
		;

	auto parsed = options.parse(argc, args);

	if (!setup(parsed)) return 1;

	while (true) {
		update();
	}

}