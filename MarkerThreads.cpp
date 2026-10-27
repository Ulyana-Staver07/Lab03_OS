#include "MarkerThreads.h"
#include "MarkerLogic.h"
#include <cstdlib>
#include <iostream>

DWORD WINAPI markerThreadProc(LPVOID parameter) {
    auto* context = static_cast<MarkerContext*>(parameter);
    WaitForSingleObject(context->startEvent, INFINITE);

    EnterCriticalSection(context->arrayLock);
    std::srand(static_cast<unsigned int>(context->id));
    LeaveCriticalSection(context->arrayLock);

    for (;;) {
        int index = -1;
        bool occupied = false;

        EnterCriticalSection(context->arrayLock);
        const auto size = context->values->size();
        index = static_cast<int>(indexFromRandom(static_cast<unsigned int>(std::rand()), size));

        if ((*context->values)[static_cast<std::size_t>(index)] == 0) {
            Sleep(5);
            (*context->values)[static_cast<std::size_t>(index)] = context->id;
            ++context->markedCount;
            Sleep(5);
        } else {
            occupied = true;
            context->blockedIndex = index;
        }
        LeaveCriticalSection(context->arrayLock);

        if (!occupied) {
            continue;
        }

        std::cout << "Поток marker " << context->id
                  << ": помечено элементов = " << context->markedCount
                  << ", занятый индекс = " << context->blockedIndex << std::endl;
        SetEvent(context->blockedEvent);
        WaitForSingleObject(context->commandEvent, INFINITE);

        if (context->command.load() == 1) {
            EnterCriticalSection(context->arrayLock);
            for (int& value : *context->values) {
                if (value == context->id) {
                    value = 0;
                }
            }
            LeaveCriticalSection(context->arrayLock);
            return 0;
        }
        // A continue command starts the next iteration of the marker loop.
    }
}
