#include "pch.h"
#include "rendering.h"
#include "constants.h"

namespace rendering {

	std::array<RenderJob, MAX_RENDER_JOBS> renderJobs;
	/*
		Mutex for access to job flags and job array
	*/
	std::mutex renderJobsMutex;
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
	RenderJob* tryPushJob(RenderJob&& targetJob) {

		std::lock_guard<std::mutex> guard(renderJobsMutex);
		for (auto& job : renderJobs) {
			if (job.free) {
				job = std::move(targetJob);
				jobNotification.notify_one();
				return &job;
			}
		}

		return nullptr;
	}

	void threadFunction(const char* threadName) {
		spdlog::info("{} started", threadName);
		while (!shouldQuit) {
			RenderJob* currJob = nullptr;
			spdlog::debug("{} finding job...", threadName);
			{
				std::lock_guard<std::mutex> guard(renderJobsMutex);
				for (auto& job : renderJobs) {
					if (!job.free && !job.working) {
						job.working = true;
						spdlog::debug("{} found job {}", threadName, (std::size_t) & job);
						currJob = &job;
					}
				}
			}

			if (currJob == nullptr) {
				spdlog::debug("{} didn't find job, waiting for notification...", threadName);
				std::unique_lock<std::mutex> guard(renderJobsMutex);
				jobNotification.wait(guard);
				spdlog::debug("{} received notification!", threadName);
			} else {

				//SDL_FillRect(currJob->surface.get(), nullptr, SDL_MapRGB(currJob->surface->format, 255, 255, 0));

				std::unique_lock<std::mutex>(renderJobsMutex);
				currJob->finished = true;
				spdlog::debug("{} finished job {}", threadName, (std::size_t) currJob);
			}


		}
		spdlog::info("{} quit", threadName);
	}

	void setupThreadSwarm() {
		spdlog::info("Launching thread swarm...");
		threads.emplace_back(threadFunction, "[Alpha]");
		threads.emplace_back(threadFunction, "<Betta>");
		threads.emplace_back(threadFunction, "{Gamma}");
		threads.emplace_back(threadFunction, ":Delta:");
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