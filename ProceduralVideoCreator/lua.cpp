#include "pch.h"
#include "constants.h"
#include "globals.h"

void setupLuaTasks(kaguya::State& state);

/*
	Runs the specified file in the luaState, thus updating it. It's called on the
	start of the render loop and every time the file is changed
*/
void loadLua(const std::filesystem::path& filePath, std::filesystem::file_time_type& fileLastModified) {
	spdlog::info("Loading script...");
	fileLastModified = std::filesystem::last_write_time(filePath);
	luaState("function update() end");
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

	luaState["setDimensions"].setFunction([](int w, int h)->void {
		if (w < 100 || h < 100) {
			luaState(R"(error("Dimensions too small, minimum is 100x100", 2))");
			return;
		}

		projectW = w;
		projectH = h;
	});

	luaState["setLength"].setFunction([](int length) {
		if (length < 1) {
			luaState(R"(error("Project length too small, minimus is 1s"))");
			return;
		}

		projectLength = length;
	});

	luaState["mapTime"].setFunction([](double time, double start, double duration) {
		auto frac = (time - start) / duration;
		frac = std::min(1.0, std::max(0.0, frac));
		frac = (std::sin((frac - 0.5) * 3.14159265359) + 1) / 2;
		return frac;
	});

	luaState["lerpText"].setFunction([](std::string src, std::string dest, double frac) {
		frac = std::min(1.0, std::max(0.0, frac));
		std::string ret = dest.substr(0, int((double)dest.length() * frac)) + src.substr(int((double)(src.length()) * frac));
		return ret;
	});

	luaState.setErrorHandler([](int statuscode, const char* message) ->void {
		throw kaguya::LuaException(statuscode, std::string(message));
	});

	spdlog::info("Registering tasks...");
	setupLuaTasks(luaState);

}