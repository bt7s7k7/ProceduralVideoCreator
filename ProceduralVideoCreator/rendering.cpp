#include "pch.h"
#include "rendering.h"
#include "constants.h"

namespace rendering {

	std::array<RenderJob, MAX_RENDER_JOBS> renderJobs;
	std::mutex renderJobsMutex;
	std::atomic<bool> shouldQuit;
	std::condition_variable jobNotification;
	std::vector<std::thread> threads;

	RenderJob* tryPushJob(RenderJob& targetJob) {

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
		spdlog::info("Thread {} started", threadName);
		while (!shouldQuit) {
			RenderJob* currJob = nullptr;
			spdlog::debug("Thread {} finding job...", threadName);
			{
				std::lock_guard<std::mutex> guard(renderJobsMutex);
				for (auto& job : renderJobs) {
					if (!job.free && !job.working) {
						job.working = true;
						spdlog::debug("Thread {} found job {}", threadName, (std::size_t) & job);
						currJob = &job;
					}
				}
			}

			if (currJob == nullptr) {
				spdlog::debug("Thread {} didn't find job, waiting for notification...", threadName);
				std::unique_lock<std::mutex> guard(renderJobsMutex);
				jobNotification.wait(guard);
				spdlog::debug("Thread {} received notification!", threadName);
			} else {
				std::unique_lock<std::mutex>(renderJobsMutex);
				currJob->finished = true;
				spdlog::debug("Thread {} finished job {}", threadName, (std::size_t) currJob);
			}


		}
		spdlog::info("Thread {} quit", threadName);
	}

	void setupThreadSwarm() {
		spdlog::info("Launching thread swarm...");
		threads.emplace_back(threadFunction, "Alpha");
		threads.emplace_back(threadFunction, "Beta");
		threads.emplace_back(threadFunction, "Gamma");
		threads.emplace_back(threadFunction, "Delta");
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