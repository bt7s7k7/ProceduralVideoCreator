#include "pch.h"
#include "constants.h"


kaguya::State luaState;

void loadLua(const std::filesystem::path& filePath, std::filesystem::file_time_type& fileLastModified) {
	spdlog::info("Loading script...");
	fileLastModified = std::filesystem::last_write_time(filePath);
	luaState.dofile(filePath.string());
	spdlog::info("Loaded!");
}

void createLuaBindings() {
	spdlog::info("Creating Lua bindings...");
	luaState["log"].setFunction([](std::string text) ->void {
		spdlog::info("[Lua] " + text);
	});

}