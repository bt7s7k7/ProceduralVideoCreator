#include "pch.h"
#include "constants.h"
#include "globals.h"
#include "rendering.h"

std::filesystem::file_time_type fileLastModified;

void loadLua(const std::filesystem::path& filePath, std::filesystem::file_time_type& fileLastModified);


bool updatePreview(RenderJob*& previewJob, std::vector<RenderTask>&& tasks) {
	if (previewJob != nullptr) {
		previewJob->finished = true;
		previewJob = nullptr;
	}

	previewJob = rendering::tryPushJob(RenderJob(projectW, projectH, previewScale, std::move(tasks)));

	return previewJob != nullptr;
}

bool updateLoop() {
	projectW = 1980;
	projectH = 1080;
	loadLua(filePath, fileLastModified);
	rendering::setupThreadSwarm();

	spdlog::info("Creating SDL windows...");

	sliderWindow.reset(handleSDLError(SDL_CreateWindow("Slider", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 700, 300, 0)));
	previewWindow.reset(handleSDLError(SDL_CreateWindow("Preview", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, projectW / previewScale, projectH / previewScale, 0)));

	sliderWindowRenderer.reset(handleSDLError(SDL_CreateRenderer(&*sliderWindow, -1, SDL_RENDERER_ACCELERATED)));

	spdlog::info("Event loop started");

	/*
		This points to the job rendering the preview, if the preview is not being rendered it's nullptr
	*/
	RenderJob* previewJob = nullptr;
	bool wantPreviewJob = true;

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
		if (wantPreviewJob) {  // If we want a render job we get it
			spdlog::debug("Requesting a render job for preview");
			wantPreviewJob = !updatePreview(previewJob, {});
		}
		if (previewJob) { // If we have a render job we test if it's done
			std::lock_guard lock(rendering::renderJobsMutex); // Lock access fot job flags, specificaly .finished to prevent a race condition
			spdlog::debug("Bliting rendered pixels to preview");
			SDL_BlitSurface(previewJob->surface.get(), nullptr, SDL_GetWindowSurface(previewWindow.get()), nullptr);
			SDL_UpdateWindowSurface(previewWindow.get());
			if (previewJob->finished) { // If done then we free it and forget about it
				spdlog::debug("Preview job finished, freeing");
				*previewJob = std::move(RenderJob());
				previewJob = nullptr;
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