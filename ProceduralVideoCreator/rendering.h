#pragma once
#include "pch.h"
#include "DestroyerType.h"
#include "SDLHelper.h"


struct RenderTask {
	virtual const char* GetName() = 0;
	virtual void Render(SDL_Surface* surface, double scale) = 0;
};

struct RenderJob {
	std::atomic<bool> canceled;
	std::atomic<bool> finished;
	std::atomic<bool> working;
	std::unique_ptr<SDL_Surface, DestroyerType<SDL_Surface, SDL_FreeSurface>> surface;
	std::vector<std::unique_ptr<RenderTask>> tasks;
	double scale;

	inline RenderJob() : canceled(true), finished(true), working(false), surface(), tasks(), scale(1) {};
	inline RenderJob(int w, int h, double scale_, std::vector<std::unique_ptr<RenderTask>>&& tasks_) : canceled(false), finished(false), surface(handleSDLError(SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0))), tasks(std::move(tasks_)), working(false), scale(scale_) {}
	/*inline RenderJob(RenderJob&& other) noexcept : canceled(other.canceled.load()), finished(other.canceled.load()), surface(std::move(other.surface)), tasks(std::move(other.tasks)), working(false), scale(1) {
		other.canceled = true;
		other.finished = true;
		other.working = false;
		other.surface.reset();
		other.tasks.clear();
	};

	inline RenderJob& operator=(RenderJob&& other) noexcept {
		canceled = other.canceled.load();
		finished = other.finished.load();
		working = other.working.load();

		other.canceled = true;
		other.finished = true;
		other.working = false;

		surface.reset();
		surface = std::move(other.surface);
		other.surface.reset();

		tasks.clear();
		tasks = std::move(other.tasks);
		other.tasks.clear();

		return *this;
	};

	inline void Reset() {
		canceled = true;
		finished = true;
		working = false;
		surface.reset();
		tasks.clear();
	};*/

	RenderJob(const RenderJob&) = delete;
};

namespace rendering {
	void tryPushJob(std::shared_ptr<RenderJob> targetJob);
	void setupThreadSwarm();
	void endThreadSwarm();
	extern std::mutex renderJobsMutex;

	inline Uint32 mapRGBA(SDL_PixelFormat* format, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255) {
		return 0
			| (r >> format->Rloss) << format->Rshift
			| (g >> format->Gloss) << format->Gshift
			| (b >> format->Bloss) << format->Bshift
			| (a >> format->Aloss) << format->Ashift;
	}
}