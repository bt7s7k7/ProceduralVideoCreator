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
	SDL_Point size;
	std::vector<std::unique_ptr<RenderTask>> tasks;
	double scale;
	std::string fileName;

	inline RenderJob() : canceled(true), finished(true), working(false), surface(), tasks(), scale(1) {};
	inline RenderJob(int w, int h, double scale_, std::vector<std::unique_ptr<RenderTask>>&& tasks_) : canceled(false), finished(false), surface(), size{ w,h }, tasks(std::move(tasks_)), working(false), scale(scale_) {}

	RenderJob(const RenderJob&) = delete;
};

namespace rendering {
	void pushJob(std::shared_ptr<RenderJob> targetJob);
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

	extern std::mutex sdlMutex;
}