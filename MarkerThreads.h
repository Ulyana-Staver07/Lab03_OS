#pragma once

#ifdef _WIN32
#include <windows.h>
#include <atomic>
#include <vector>

struct MarkerContext {
    int id = 0;
    std::vector<int>* values = nullptr;
    CRITICAL_SECTION* arrayLock = nullptr;
    HANDLE startEvent = nullptr;
    HANDLE blockedEvent = nullptr;
    HANDLE commandEvent = nullptr;
    HANDLE threadHandle = nullptr;
    std::atomic<int> command{0}; // 0 = continue, 1 = stop
    int blockedIndex = -1;
    std::size_t markedCount = 0;
};

DWORD WINAPI markerThreadProc(LPVOID parameter);
#endif
