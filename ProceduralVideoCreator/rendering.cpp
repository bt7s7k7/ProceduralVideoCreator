#include "pch.h"
#include "rendering.h"
#include "constants.h"

namespace rendering {

	std::queue<std::shared_ptr<RenderJob>> renderJobs;
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
	void tryPushJob(std::shared_ptr<RenderJob> targetJob) {
		std::lock_guard<std::mutex> guard(renderJobsMutex);
		renderJobs.push(targetJob);
		jobNotification.notify_one();
		spdlog::debug("Pushed job, now {} jobs in queue", renderJobs.size());
	}

	void threadFunction(const char* threadName) {

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
				//SDL_LockSurface(currJob->surface.get());
				auto surface = currJob->surface.get();
				for (auto& task : currJob->tasks) {
					task->Render(surface, currJob->scale);
					std::lock_guard<std::mutex> l(rendering::renderJobsMutex);
					if (currJob->canceled) break;
				}
				std::lock_guard<std::mutex> l(rendering::renderJobsMutex);
				if (currJob->canceled) {
					spdlog::debug("{}'s job {} was cancelled", threadName, (std::size_t) currJob.get());
				} else {
					spdlog::debug("{} finished job {}", threadName, (std::size_t) currJob.get());
					currJob->finished = true;
				}
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