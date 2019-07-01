#pragma once

constexpr char PROGRAM_NAME[] = "ProceduralVideoCreator";

constexpr char FILE_ARG_NAME[] = "file";
constexpr char FILE_ARG_DEFINITION[] = "f,file";
constexpr char FILE_ARG_DESC[] = "Lua file to execute";

constexpr char NOHOTSWAP_ARG_NAME[] = "c";
constexpr char NOHOTSWAP_ARG_DEFINITION[] = "c,cold";
constexpr char NOHOTSWAP_ARG_DESC[] = "Disable hotswap/live coding";

constexpr char DEBUG_ARG_NAME[] = "debug";
constexpr char DEBUG_ARG_DEFINITION[] = "d,debug";
constexpr char DEBUG_ARG_DESC[] = "Increased logging verbosity level to debug";

constexpr std::size_t MAX_RENDER_JOBS = 255;