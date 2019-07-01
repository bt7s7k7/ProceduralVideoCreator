#include "pch.h"
#include "constants.h"
#include "globals.h"
#include "rendering.h"

std::filesystem::file_time_type fileLastModified;

void loadLua(const std::filesystem::path& filePath, std::filesystem::file_time_type& fileLastModified);

bool updateLoop() {
	loadLua(filePath, fileLastModified);
	rendering::setupThreadSwarm();
	spdlog::info("Event loop started");
	while (true) {
		SDL_Event event;
		// Polling and reacting to SDL events
		while (SDL_PollEvent(&event)) { 
			if (event.type == SDL_WINDOWEVENT) {
				if (event.window.event == SDL_WINDOWEVENT_CLOSE) { // Testing if any window was closed, triggering a soft exit
					goto end;
				}
			}
		}
	}

end:
	spdlog::info("Update loop ended, cleaning up...");
	// Destroying all SDL resources
	sliderWindowRenderer.reset();
	sliderWindow.reset();
	previewWindow.reset();

	rendering::endThreadSwarm();

	return true;
}