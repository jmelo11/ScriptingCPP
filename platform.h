#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdint>
#include <thread>

using DWORD = std::uint32_t;
using LPSTR = char*;

inline DWORD GetCurrentThreadId() {
    std::hash<std::thread::id> hasher;
    return static_cast<DWORD>(hasher(std::this_thread::get_id()));
}

#endif

#endif // PLATFORM_H
