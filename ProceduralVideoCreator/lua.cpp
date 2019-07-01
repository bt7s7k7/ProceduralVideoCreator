#include "pch.h"
#include "constants.h"

extern std::filesystem::file_time_type fileLastModified;
extern std::filesystem::path filePath;

kaguya::State luaState;

void loadLua() {
	spdlog::info("Loading script...");
	fileLastModified = std::filesystem::last_write_time(filePath);
	luaState.dofile(filePath.string());
	spdlog::info("Loaded!");
}

void createLuaBindings() {

}