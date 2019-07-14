#include "pch.h"
#include "rendering.h"
#include "constants.h"

namespace rendering {

	std::queue<std::shared_ptr<RenderJob>> renderJobs;
	/*
		Mutex for access to job flags and job array
	*/
	std::mutex renderJobsMutex;
	std::mutex sdlMutex;
	/*
		This value is checked by all thrads every iteration
		If set to true all threads will return
	*/
	std::atomic<bool> shouldQuit;
	/*
		It's triggered when a new job is pushed or when the
		swarm should return. All threads wait for it without
		timeout. Accidental trigger will have no effect
	*/
	std::condition_variable jobNotification;
	/*
		Lists all thread in the swarm.
	*/
	std::vector<std::thread> threads;

	/*
		Tries to add a job to renderJobs array. If all jobs
		are allocated returns nullptr. If the job is pushed
		returns a pointer to the new location. This pointer
		should be checked by the caller to find out if the
		job was completed, then the job should be freed by
		using the assign operator with a defaultly constructed
		job.
	*/
	void pushJob(std::shared_ptr<RenderJob> targetJob) {
		std::lock_guard<std::mutex> guard(renderJobsMutex);
		renderJobs.push(targetJob);
		jobNotification.notify_one();
		spdlog::debug("Pushed job, now {} jobs in queue", renderJobs.size());
	}

	void threadFunction(std::string threadName) {

		spdlog::info("{} started", threadName);
		while (!shouldQuit) {
			std::shared_ptr<RenderJob> currJob = nullptr;
			spdlog::debug("{} finding job...", threadName);
			{
				std::lock_guard<std::mutex> guard(renderJobsMutex);
				if (renderJobs.size() > 0) {
					currJob = renderJobs.front();
					renderJobs.pop();
					spdlog::debug("{} found job {}", threadName, (std::size_t) currJob.get());
				}
			}

			if (currJob == nullptr) {
				spdlog::debug("{} didn't find job, waiting for notification...", threadName);
				std::unique_lock<std::mutex> guard(renderJobsMutex);
				jobNotification.wait(guard);
				spdlog::debug("{} received notification!", threadName);
			} else {
				std::unique_lock<std::mutex>(renderJobsMutex);
				{ // Allocate surface
					SDL_Surface* surface = nullptr;
					while (surface == nullptr) {
						{
							std::lock_guard<std::mutex> l(sdlMutex);
							surface = SDL_CreateRGBSurface(0, currJob->size.x, currJob->size.y, 32, 0, 0, 0, 0);
						}
						SDL_Delay(10); // If not enough space, wait
					}
					currJob->surface.reset(surface);
				}
				auto surface = currJob->surface.get();
				for (auto& task : currJob->tasks) {
					task->Render(surface, currJob->scale);
					std::lock_guard<std::mutex> l(rendering::renderJobsMutex);
					if (currJob->canceled) break;
				}
				std::unique_lock<std::mutex> l(rendering::renderJobsMutex);
				if (currJob->canceled) {
					spdlog::debug("{}'s job {} was cancelled", threadName, (std::size_t) currJob.get());
				} else {
					if (!currJob->fileName.empty()) {
						l.unlock();
						IMG_SaveJPG(surface, currJob->fileName.data(), 100);
						l.lock();
					}

					spdlog::debug("{} finished job {}", threadName, (std::size_t) currJob.get());
					currJob->finished = true;
				}
			}


		}
		spdlog::info("{} quit", threadName);
	}

	void setupThreadSwarm() {
		spdlog::info("Launching thread swarm...");
		for (int i = 0; i < 10; i++) {
			auto name = ("[" + std::string(1, 'A' + i) + "]");
			threads.emplace_back(threadFunction, name);
		}
	}

	void endThreadSwarm() {
		spdlog::info("Ending thread swarm...");
		shouldQuit = true;
		jobNotification.notify_all();
		for (auto& thread : threads) {
			thread.join();
		}
	}
}