#include "pch.h"
#include "constants.h"
#include "globals.h"
#include "rendering.h"
#include "font.h"

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
	projectLength = 5;
	loadLua(filePath, fileLastModified);
	rendering::setupThreadSwarm();
	auto font = getOrLoadFont(16, "segoeui");

	spdlog::info("Creating SDL windows...");

	sliderWindow.reset(handleSDLError(SDL_CreateWindow("Slider", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, CONTROLS_PADDING + CONTROLS_COUNT * (CONTROLS_WIDTH + CONTROLS_PADDING), CONTROLS_PADDING + 2 * (CONTROLS_HEIGHT + CONTROLS_PADDING), 0)));
	previewWindow.reset(handleSDLError(SDL_CreateWindow("Preview", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, projectW / previewScale, projectH / previewScale, 0)));

	sliderWindowRenderer.reset(handleSDLError(SDL_CreateRenderer(&*sliderWindow, -1, SDL_RENDERER_ACCELERATED)));

	spdlog::info("Event loop started");

	/*
		This points to the job rendering the preview, if the preview is not being rendered it's nullptr
	*/
	RenderJob* previewJob = nullptr;
	bool wantPreviewJob = true;
	double time = 0;

	struct buttonState_t {
		bool over;
		bool pressed;
		bool wasPressed;

		inline SDL_Color GetColor() {
			return pressed ? CONTROL_ACTIVE_COLOR : (over ? CONTROL_HOVER_COLOR : CONTROL_IDLE_COLOR);
		};

		buttonState_t() : over(false), pressed(false), wasPressed(false) {};
	};

	Uint32 lastMouseState = SDL_GetMouseState(nullptr, nullptr);

	while (true) {

		// Detecting file change
		if (!ignoreChanges) {
			try {
				if (std::filesystem::last_write_time(filePath) > fileLastModified) {
					try {
						loadLua(filePath, fileLastModified);
						spdlog::info("Reloaded specified file");
						wantPreviewJob = true;
					} catch (const kaguya::LuaException& err) {
						spdlog::warn("Exception occured while executing specified file: \n{}", err.what());
						fileLastModified = std::filesystem::last_write_time(filePath);
					}
				}
			} catch (const std::filesystem::filesystem_error& err) {
				spdlog::warn("Exception '{}' occured while reading specified file", err.what());

			}
		}

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

		{ // Render slider window controlls
			auto renderer = sliderWindowRenderer.get();

			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			SDL_RenderFillRect(renderer, nullptr);

			auto renderText = [&font, &renderer](const std::string& text, int x, int y, SDL_Color color = CONTROL_LABEL_COLOR) {
				renderCopySurfaceAndFree(renderer, handleSDLError(TTF_RenderText_Blended(font, text.data(), color)), x, y);
			};

			auto fillRect = [&renderer](int x, int y, int w, int h, SDL_Color color = CONTROL_IDLE_COLOR) {
				SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
				SDL_Rect rect{ x,y,w,h };
				SDL_RenderFillRect(renderer, &rect);
			};
			int mouseX, mouseY;
			auto mouseState = SDL_GetMouseState(&mouseX, &mouseY);
			auto drawControll = [&renderer, &font, pos = CONTROLS_PADDING, &fillRect, &renderText, &lastMouseState, &mouseState, &mouseX, &mouseY](const char* text, bool ignoreMouse = false) mutable {

				bool over = mouseX >= pos && mouseX < pos + CONTROLS_WIDTH && mouseY >= CONTROLS_PADDING && mouseY < CONTROLS_PADDING + CONTROLS_HEIGHT;
				bool active = over && (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) > 0;
				bool pressed = active && (lastMouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) == 0;


				fillRect(pos, CONTROLS_PADDING, CONTROLS_WIDTH, CONTROLS_HEIGHT, active && !ignoreMouse ? CONTROL_ACTIVE_COLOR : (over && !ignoreMouse ? CONTROL_HOVER_COLOR : CONTROL_IDLE_COLOR));
				renderText(text, pos + CONTROLS_PADDING, 0);
				pos += CONTROLS_PADDING + CONTROLS_WIDTH;

				return pressed;
			};

			drawControll(fmt::format(LABEL_TIME, time, projectLength).data(), true);
			drawControll(LABEL_PAUSEPLAY);
			drawControll(LABEL_RENDERFRAME);
			drawControll(LABEL_RENDER_PROJECT);
			drawControll(LABEL_FORCERELOAD);

			{ // Slider controll
				if (mouseX >= CONTROLS_PADDING && mouseX < CONTROLS_COUNT * (CONTROLS_WIDTH + CONTROLS_PADDING) && mouseY >= CONTROLS_PADDING * 2 + CONTROLS_HEIGHT && mouseY < (CONTROLS_PADDING + CONTROLS_HEIGHT) * 2 && (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) > 0) {
					double frac = (double(mouseX) - CONTROLS_PADDING) / (double(CONTROLS_COUNT * (CONTROLS_WIDTH + CONTROLS_PADDING)) - CONTROLS_PADDING);
					time = frac * projectLength;
				}

				fillRect(CONTROLS_PADDING, CONTROLS_PADDING * 2 + CONTROLS_HEIGHT + CONTROLS_HEIGHT / 2 - SLIDER_LINE_WIDTH / 2, (CONTROLS_COUNT * (CONTROLS_WIDTH + CONTROLS_PADDING)) - CONTROLS_PADDING, SLIDER_LINE_WIDTH);
				fillRect(static_cast<int>(CONTROLS_PADDING + ((time / projectLength) * (CONTROLS_COUNT * (CONTROLS_WIDTH + CONTROLS_PADDING)) - CONTROLS_PADDING)), CONTROLS_PADDING * 2 + CONTROLS_HEIGHT, SLIDER_HANDLE_WIDTH, CONTROLS_HEIGHT, CONTROL_HOVER_COLOR);
			}

			lastMouseState = SDL_GetMouseState(nullptr, nullptr);

			SDL_RenderPresent(renderer);
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