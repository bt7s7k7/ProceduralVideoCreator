// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here

#include <SDL2/SDL.h>
#undef main
#include "kaguya.h"
#define SPDLOG_DISABLE_TID_CACHING 1
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <cxxopts.hpp>
#include <SDL2/SDL_ttf.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <SDL2/SDL_image.h>

#include <iostream>
#include <filesystem>
#include <chrono>
#include <exception>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <new>
#include <queue>

#endif //PCH_H
