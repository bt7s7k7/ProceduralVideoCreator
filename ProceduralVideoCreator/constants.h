#pragma once

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

constexpr char SCALE_ARG_NAME[] = "scale";
constexpr char SCALE_ARG_DEFINITION[] = "s,scale";
constexpr char SCALE_ARG_DESC[] = "Scaling of the preview relative to desired output (If --scale=2 then width is --width/2)";

constexpr std::size_t MAX_RENDER_JOBS = 255;