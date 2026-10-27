#include "MarkerThreads.h"
#include "MarkerLogic.h"
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

namespace {
void printArray(const std::vector<int>& values, CRITICAL_SECTION& lock) {
    EnterCriticalSection(&lock);
    std::cout << "Массив: ";
    for (std::size_t i = 0; i < values.size(); ++i) {
        std::cout << '[' << i << "]=" << values[i] << ' ';
    }
    std::cout << std::endl;
    LeaveCriticalSection(&lock);
}

int readPositive(const char* prompt, int maximum = 100000) {
    int value;
    for (;;) {
        std::cout << prompt;
        if (std::cin >> value && value > 0 && value <= maximum) return value;
        std::cout << "Введите целое число от 1 до " << maximum << ".\n";
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    }
}
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    const int arraySize = readPositive("Введите размер массива: ", 100000);
    const int markerCount = readPositive("Введите количество потоков marker (1–60): ", 60);
    std::vector<int> values(static_cast<std::size_t>(arraySize), 0);

    CRITICAL_SECTION arrayLock;
    InitializeCriticalSection(&arrayLock);
    HANDLE startEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!startEvent) {
        std::cerr << "Не удалось создать событие запуска.\n";
        DeleteCriticalSection(&arrayLock);
        return 1;
    }

    std::vector<std::unique_ptr<MarkerContext>> markers;
    markers.reserve(static_cast<std::size_t>(markerCount));
    bool creationFailed = false;

    for (int id = 1; id <= markerCount; ++id) {
        auto context = std::make_unique<MarkerContext>();
        context->id = id;
        context->values = &values;
        context->arrayLock = &arrayLock;
        context->startEvent = startEvent;
        context->blockedEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        context->commandEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        if (!context->blockedEvent || !context->commandEvent) {
            std::cerr << "Не удалось создать события для потока " << id << ".\n";
            if (context->blockedEvent) CloseHandle(context->blockedEvent);
            if (context->commandEvent) CloseHandle(context->commandEvent);
            creationFailed = true;
            break;
        }
        context->threadHandle = CreateThread(nullptr, 0, markerThreadProc, context.get(), 0, nullptr);
        if (!context->threadHandle) {
            std::cerr << "Не удалось создать поток " << id << ".\n";
            CloseHandle(context->blockedEvent);
            CloseHandle(context->commandEvent);
            creationFailed = true;
            break;
        }
        markers.push_back(std::move(context));
    }

    SetEvent(startEvent);
    std::vector<bool> active(markers.size(), true);

    while (!markers.empty()) {
        int activeCount = 0;
        for (bool isActive : active) if (isActive) ++activeCount;
        if (activeCount == 0) break;

        for (std::size_t i = 0; i < markers.size(); ++i) {
            if (active[i]) WaitForSingleObject(markers[i]->blockedEvent, INFINITE);
        }

        printArray(values, arrayLock);
        std::cout << "Активные потоки: ";
        for (std::size_t i = 0; i < markers.size(); ++i) {
            if (active[i]) std::cout << markers[i]->id << ' ';
        }
        std::cout << '\n';

        int chosen = 0;
        for (;;) {
            std::cout << "Введите номер marker для завершения: ";
            if (std::cin >> chosen) {
                bool found = false;
                for (std::size_t i = 0; i < markers.size(); ++i)
                    if (active[i] && markers[i]->id == chosen) found = true;
                if (found) break;
            }
            std::cout << "Такого активного потока нет. Попробуйте ещё раз.\n";
            std::cin.clear();
            std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        }

        std::size_t chosenIndex = 0;
        for (; chosenIndex < markers.size(); ++chosenIndex)
            if (markers[chosenIndex]->id == chosen) break;

        for (std::size_t i = 0; i < markers.size(); ++i)
            if (active[i]) ResetEvent(markers[i]->blockedEvent);

        markers[chosenIndex]->command.store(1);
        SetEvent(markers[chosenIndex]->commandEvent);
        WaitForSingleObject(markers[chosenIndex]->threadHandle, INFINITE);
        active[chosenIndex] = false;
        std::cout << "Поток marker " << chosen << " завершён; его элементы очищены.\n";
        printArray(values, arrayLock);

        for (std::size_t i = 0; i < markers.size(); ++i) {
            if (active[i]) {
                markers[i]->command.store(0);
                SetEvent(markers[i]->commandEvent);
            }
        }
    }

    for (auto& marker : markers) {
        if (marker->threadHandle) CloseHandle(marker->threadHandle);
        if (marker->blockedEvent) CloseHandle(marker->blockedEvent);
        if (marker->commandEvent) CloseHandle(marker->commandEvent);
    }
    CloseHandle(startEvent);
    DeleteCriticalSection(&arrayLock);
    if (creationFailed) std::cerr << "Работа завершена после ошибки создания потока/события.\n";
    std::cout << "Все потоки marker завершили работу.\n";
    return creationFailed ? 1 : 0;
}
