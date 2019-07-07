#pragma once
#include "pch.h"
#include "DestroyerType.h"
#include "SDLHelper.h"


struct RenderTask {
	virtual const char* GetName() = 0;
	virtual void Render(SDL_Surface * surface, int scale) = 0;
};
 
struct RenderJob {
	std::atomic<bool> free;
	std::atomic<bool> finished;
	std::atomic<bool> working;
	std::unique_ptr<SDL_Surface, DestroyerType<SDL_Surface, SDL_FreeSurface>> surface;
	std::vector<RenderTask> tasks;
	int scale;

	inline RenderJob() : free(true), finished(true), working(false), surface(), tasks(), scale(1) {};
	inline RenderJob(int w, int h, int scale_, std::vector<RenderTask>&& tasks_) : free(false), finished(false), surface(handleSDLError(SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0))), tasks(std::move(tasks_)), working(false), scale(scale_) {}
	inline RenderJob(RenderJob&& other) noexcept : free(other.free.load()), finished(other.free.load()), surface(std::move(other.surface)), tasks(std::move(other.tasks)), working(false), scale(1) {
		other.finished = true;
	} ;

	inline RenderJob & operator=(RenderJob&& other) noexcept {
		this->~RenderJob();
		new (this) RenderJob(std::move(other));
		return *this;
	}

	RenderJob(const RenderJob&) = delete;
};

namespace rendering {
	RenderJob* tryPushJob(RenderJob&& targetJob);
	void setupThreadSwarm();
	void endThreadSwarm();
	extern std::mutex renderJobsMutex;
}