#include "pch.h"
#include "constants.h"

/*
	State of the Lua VM. Doesn't change
	during the program runtime
*/
kaguya::State luaState;

/*
	Runs the specified file in the luaState, thus updating it. It's called on the
	start of the render loop and every time the file is changed
*/
void loadLua(const std::filesystem::path& filePath, std::filesystem::file_time_type& fileLastModified) {
	spdlog::info("Loading script...");
	fileLastModified = std::filesystem::last_write_time(filePath);
	luaState.dofile(filePath.string());
	spdlog::info("Loaded!");
}

/* 
	Creates bindings in Lua to C++ functions.
*/
void createLuaBindings() {
	spdlog::info("Creating Lua bindings...");
	luaState["log"].setFunction([](std::string text) ->void {
		spdlog::info("[Lua] " + text);
	});

	luaState.setErrorHandler([](int statuscode, const char* message) ->void {
		throw kaguya::LuaException(statuscode, std::string(message));
	});

}