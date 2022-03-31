#pragma once

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#include "win32/Frontend.hpp"
namespace m2j {
typedef win32::Frontend CurrentFrontend;
}
#elif defined(__linux__)
// #include "linux/Frontend.hpp"
// namespace m2j { typedef linux::Frontend CurrentFrontend; }
static_assert(false, "Linux is not supported (yet)");
#elif defined(__APPLE__) || defined(__MACH__)
// #include "macos/Frontend.hpp"
// namespace m2j { typedef macos::Frontend CurrentFrontend; }
static_assert(false, "macOS is not supported (yet)");
#endif
