#include "pch.h"
#include "constants.h"
#include "globals.h"
#include "rendering.h"
#include "font.h"

std::filesystem::file_time_type fileLastModified;

void loadLua(const std::filesystem::path& filePath, std::filesystem::file_time_type& fileLastModified);
std::vector<std::unique_ptr<RenderTask>> updateLua(double time);

/*
	This function pushes a render job to the stack and saves it to previewJob,
	if a job already existed it will be canceled
*/
void updatePreview(std::shared_ptr<RenderJob>& previewJob, std::vector<std::unique_ptr<RenderTask>>&& tasks) {
	if (previewJob) {
		std::lock_guard<std::mutex> l(rendering::renderJobsMutex);
		previewJob->canceled = true;
		previewJob.reset();
	}
	previewJob = std::make_shared<RenderJob>(projectW, projectH, previewScale, std::move(tasks));
	rendering::tryPushJob(previewJob);
}

bool updateLoop() {
	rendering::setupThreadSwarm();
	auto font = getOrLoadFont(16, "segoeui"); // Loading font
	std::unordered_map<std::string, unique_surface_ptr> cachedLabels;

	spdlog::info("Creating SDL windows...");

	// Creating the slider window, sized based on control size constants
	sliderWindow.reset(handleSDLError(SDL_CreateWindow("Slider", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, CONTROLS_PADDING + CONTROLS_COUNT * (CONTROLS_WIDTH + CONTROLS_PADDING), CONTROLS_PADDING + 2 * (CONTROLS_HEIGHT + CONTROLS_PADDING), 0)));
	// Creating the preview window, will be resized automaticaly based on Lua later
	previewWindow.reset(handleSDLError(SDL_CreateWindow("Preview", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 10, 10, 0)));
	// Creating the renderer for slider window it's used for drawing controls 
	sliderWindowRenderer.reset(handleSDLError(SDL_CreateRenderer(&*sliderWindow, -1, SDL_RENDERER_ACCELERATED)));

	/*
		This function recalculates the size of the preview
		window based on projectSize and maxPreviewSize,
		used after reseting Lua and once before loop
	*/
	auto recalcPreviewScale = []() {
		if (projectH > maxPreviewSize || projectW > maxPreviewSize) {
			double got = std::max(projectH, projectW);
			double need = maxPreviewSize;
			previewScale = need / got;
		} else {
			previewScale = 1;
		}

		SDL_SetWindowSize(previewWindow.get(), static_cast<int>(projectW * previewScale), static_cast<int>(projectH * previewScale));
	};

	spdlog::info("Event loop started");

	/*
		This points to the job rendering the preview, if the preview is not being rendered it's nullptr
	*/
	std::shared_ptr<RenderJob> previewJob = nullptr;
	/* 
		Dictates if a preview job is wanted, set on Lua reset,
		time scrubbing, and during plaing. It's checked inside
		loop. When set it's reset and a preview job is requested 
	*/
	bool wantPreviewJob = true;
	/* 
		Current time of preview,
		it's set by plaing and 
		time scrubbing. It's checked
		by time scrubbing and plaing
		code and when Lua resets.
	*/
	double time = 0;
	/*
		Time last frame. Set and used
		by time scrubbing code.
	*/
	double lastTime = 0;
	/*
		Was slider pressed last frame? Set
		and used by time scrubbing code.
	*/
	bool wasSliderDown = false;
	/*
		Is preview currently plaing? Set by controls code and
		by plaing code (only sets to false when end of project
		reached). Checked by plaing code, which only increments
		time if active.
	*/
	bool plaing = false;
	/*
		Time when last frame happend while preview plaing.
		Used and set by plaing code. Difference between this
		and current time is used to increment preview time.
	*/
	std::chrono::high_resolution_clock::time_point lastPlayFrame;

	/* 
		Last state of mouse buttons. Used and set by 
		controls code. Used for click detection.
	*/
	Uint32 lastMouseState = SDL_GetMouseState(nullptr, nullptr);

	while (true) {

		// Detecting file change
		if (!ignoreChanges) { // Check if cold mode is activated using --cold
			try {
				if (std::filesystem::last_write_time(filePath) > fileLastModified) {
					try {
						// Defaults to be optionaly overwritten by Lua
						projectW = 1980;
						projectH = 1080;
						projectLength = 5;
						// Reloading Lua
						loadLua(filePath, fileLastModified);
						spdlog::info("Reloaded specified file");
						// Want preview job to see rerender preview using new Lua code
						wantPreviewJob = true;

					} catch (const kaguya::LuaException& err) { // Trigger on exceptions inside Lua so the whole program doesn't crash 
						spdlog::warn("Exception occured while executing specified file: \n{}", err.what());
					}
					// Recalculating preview scale for new dimensions optionaly set by Lua
					recalcPreviewScale();
					// Checking if time is inside the length optionaly set by Lua
					if (time > projectLength) time = projectLength;
				}
			} catch (const std::filesystem::filesystem_error& err) { // Trigger on file reading error to inform user. Can potetialy happen randomly.
				spdlog::warn("Exception '{}' occured while reading specified file", err.what());
			}
		}

		SDL_Event event; // Event to store the polled events
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
			wantPreviewJob = false;
			try {
				updatePreview(previewJob, updateLua(time));
			} catch (const kaguya::LuaException& err) { // Trigger on exceptions inside Lua so the whole program doesn't crash 
				spdlog::warn("Exception occured while executing update: \n{}", err.what());
				luaState["update"].setFunction([]() {}); // Reset the update function so it doesn't error every time we try to update preview.
			}
		}
		if (previewJob) { // If we have a render job we test if it's done
			SDL_BlitSurface(previewJob->surface.get(), nullptr, SDL_GetWindowSurface(previewWindow.get()), nullptr); // Blit finished pixels to preview
			SDL_UpdateWindowSurface(previewWindow.get());
			if (previewJob->finished) { // If done then we free it and forget about it
				spdlog::debug("Preview job finished, freeing");
				previewJob.reset();
			}
		}

		{ // Render slider window controlls
			auto renderer = sliderWindowRenderer.get();

			// Drawing background
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			SDL_RenderFillRect(renderer, nullptr); 

			auto renderText = [&font, &renderer, &cachedLabels](const std::string& text, int x, int y, bool dontChache, SDL_Color color = CONTROL_LABEL_COLOR) {
				if (dontChache)	renderCopySurfaceAndFree(renderer, handleSDLError(TTF_RenderText_Blended(font, text.data(), color)), x, y);
				else {
					auto iter = cachedLabels.find(text);
					if (iter == cachedLabels.end()) {
						auto ret = cachedLabels.insert_or_assign(text, unique_surface_ptr(handleSDLError(TTF_RenderText_Blended(font, text.data(), color))));
						iter = ret.first;
					}
					renderCopySurface(renderer, iter->second.get(), x, y);
				}
			};

			auto fillRect = [&renderer](int x, int y, int w, int h, SDL_Color color = CONTROL_IDLE_COLOR) {
				SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
				SDL_Rect rect{ x,y,w,h };
				SDL_RenderFillRect(renderer, &rect);
			};
			// Getting the mouse state to test if buttons are hovered upon or clicked
			int mouseX, mouseY;
			auto mouseState = SDL_GetMouseState(&mouseX, &mouseY);
			/*
				This statefull lambda is used to draw controll one after another separated by padding
				automatically. It also checks for hovers and clicks. Returns true if clicked.
			*/
			auto drawControll = [&renderer, &font, pos = CONTROLS_PADDING, &fillRect, &renderText, &lastMouseState, &mouseState, &mouseX, &mouseY](const char* text, bool ignoreMouse = false) mutable {

				bool over = mouseX >= pos && mouseX < pos + CONTROLS_WIDTH && mouseY >= CONTROLS_PADDING && mouseY < CONTROLS_PADDING + CONTROLS_HEIGHT;
				bool active = over && (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) > 0;
				bool pressed = active && (lastMouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) == 0;


				fillRect(pos, CONTROLS_PADDING, CONTROLS_WIDTH, CONTROLS_HEIGHT, active && !ignoreMouse ? CONTROL_ACTIVE_COLOR : (over && !ignoreMouse ? CONTROL_HOVER_COLOR : CONTROL_IDLE_COLOR));
				renderText(text, pos + CONTROLS_PADDING, 0, ignoreMouse);
				pos += CONTROLS_PADDING + CONTROLS_WIDTH;

				return pressed;
			};

			drawControll(fmt::format(LABEL_TIME, time, projectLength).data(), true);
			if (drawControll(LABEL_PAUSEPLAY)) { // Pausing a plaing the preview
				if (plaing) {
					plaing = false;
					wantPreviewJob = true; // If we stop plaing, we request a preview job to make sure to render the last frame
				} else {
					plaing = true;
					lastPlayFrame = std::chrono::high_resolution_clock::now(); // Update the last frame time point, used to increment preview time in plaing code
				}
			}
			drawControll(LABEL_RENDERFRAME);
			drawControll(LABEL_RENDER_PROJECT);
			drawControll(LABEL_FORCERELOAD);

			{ // Slider controll
				// Testing if mouse is over the slider and left mouse button is down
				if (mouseX >= CONTROLS_PADDING && mouseX < CONTROLS_COUNT * (CONTROLS_WIDTH + CONTROLS_PADDING) && mouseY >= CONTROLS_PADDING * 2 + CONTROLS_HEIGHT && mouseY < (CONTROLS_PADDING + CONTROLS_HEIGHT) * 2 && (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) > 0) {
					double frac = (double(mouseX) - CONTROLS_PADDING) / (double(CONTROLS_COUNT * (CONTROLS_WIDTH + CONTROLS_PADDING)) - CONTROLS_PADDING); // Calculate the percentage of the slider before the mouse
					time = frac * projectLength; // Setting the time based on mouse
					if (time != lastTime) { // If the time changed we update the preview
						if (!previewJob) { // We update the preview only if a job is not active to prevent canceling it and to prevent flashing of preview
							wantPreviewJob = true;
						}
					}
					wasSliderDown = true;
					lastTime = time;
				} else {
					if (wasSliderDown) { // When the slider is released we update the preview to make sure the last frame is rendered
						wantPreviewJob = true;
					}
					wasSliderDown = false;
				}

				// Plaing code
				if (plaing) {
					auto now = std::chrono::high_resolution_clock::now();
					double elapsedTime = std::chrono::duration<double>((now - lastPlayFrame)).count();
					lastPlayFrame = now; // Calculate the difference between last frame and now to increment time by real elapsed time
					time += elapsedTime;
					if (time > projectLength) { // If time reaches end we stop plaing
						time = projectLength;
						plaing = false;
						wantPreviewJob = true;
					}
					if (!previewJob) { // We update the preview only if a job is not active to prevent canceling it and to prevent flashing of preview
						wantPreviewJob = true;
					}
				}

				// Drawing the slider
				fillRect(CONTROLS_PADDING, CONTROLS_PADDING * 2 + CONTROLS_HEIGHT + CONTROLS_HEIGHT / 2 - SLIDER_LINE_WIDTH / 2, (CONTROLS_COUNT * (CONTROLS_WIDTH + CONTROLS_PADDING)) - CONTROLS_PADDING, SLIDER_LINE_WIDTH); // Slider line
				fillRect(static_cast<int>(CONTROLS_PADDING + ((time / projectLength) * (CONTROLS_COUNT * (CONTROLS_WIDTH + CONTROLS_PADDING)) - CONTROLS_PADDING)), CONTROLS_PADDING * 2 + CONTROLS_HEIGHT, SLIDER_HANDLE_WIDTH, CONTROLS_HEIGHT, CONTROL_HOVER_COLOR); // Slider handle
			}

			// Setting the last mouse state
			lastMouseState = SDL_GetMouseState(nullptr, nullptr);
			// Presenting the rendered content to slider window
			SDL_RenderPresent(renderer);
		}
	}

end:
	spdlog::info("Update loop ended, cleaning up...");
	// Destroying all SDL resources
	_renderCopySurfaceCachedTextures.clear();
	sliderWindowRenderer.reset();
	sliderWindow.reset();
	previewWindow.reset();

	rendering::endThreadSwarm();

	return true;
}