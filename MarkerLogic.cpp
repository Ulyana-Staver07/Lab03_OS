#include "MarkerLogic.h"
#include <stdexcept>

std::size_t indexFromRandom(unsigned int randomValue, std::size_t arraySize) {
    if (arraySize == 0) {
        throw std::invalid_argument("Array size must be greater than zero");
    }
    return static_cast<std::size_t>(randomValue) % arraySize;
}

std::size_t countMarkedBy(const std::vector<int>& values, int markerId) {
    std::size_t count = 0;
    for (int value : values) {
        if (value == markerId) {
            ++count;
        }
    }
    return count;
}

void clearMarkedBy(std::vector<int>& values, int markerId) {
    for (int& value : values) {
        if (value == markerId) {
            value = 0;
        }
    }
}
