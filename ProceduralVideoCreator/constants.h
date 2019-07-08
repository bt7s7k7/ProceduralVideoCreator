#pragma once
#include "pch.h"

constexpr char PROGRAM_NAME[] = "ProceduralVideoCreator";

constexpr char FILE_ARG_NAME[] = "file";
constexpr char FILE_ARG_DEFINITION[] = "f,file";
constexpr char FILE_ARG_DESC[] = "Lua file to execute";

constexpr char NOHOTSWAP_ARG_NAME[] = "cold";
constexpr char NOHOTSWAP_ARG_DEFINITION[] = "c,cold";
constexpr char NOHOTSWAP_ARG_DESC[] = "Disable hotswap/live coding";

constexpr char DEBUG_ARG_NAME[] = "debug";
constexpr char DEBUG_ARG_DEFINITION[] = "d,debug";
constexpr char DEBUG_ARG_DESC[] = "Increased logging verbosity level to debug";

constexpr char MAXS_ARG_NAME[] = "max_size";
constexpr char MAXS_ARG_DEFINITION[] = "m,max_size";
constexpr char MAXS_ARG_DESC[] = "Maximum size of preview";

constexpr std::size_t MAX_RENDER_JOBS = 4;

constexpr int CONTROLS_PADDING = 4, CONTROLS_LABEL_PADDING = 2, CONTROLS_HEIGHT = 20, CONTROLS_FONT_SIZE = CONTROLS_HEIGHT - CONTROLS_LABEL_PADDING * 2, SLIDER_LINE_WIDTH = 2, SLIDER_HANDLE_WIDTH = 4, CONTROLS_COUNT = 5, CONTROLS_WIDTH = 80;
constexpr char LABEL_TIME[] = "{:.2f} / {}", LABEL_PAUSEPLAY[] = "Pause/Play", LABEL_RENDERFRAME[] = "Frame", LABEL_RENDER_PROJECT[] = "Render", LABEL_FORCERELOAD[] = "Force";
constexpr SDL_Color CONTROL_IDLE_COLOR = { 50,50,50,255 }, CONTROL_HOVER_COLOR = { 100,100,100,255 }, CONTROL_ACTIVE_COLOR = { 50,100,50,255 }, CONTROL_LABEL_COLOR = {255,255,255,255};