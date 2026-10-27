#pragma once
#include <cstddef>
#include <vector>

// Converts a non-negative random value into a valid array index.
std::size_t indexFromRandom(unsigned int randomValue, std::size_t arraySize);

// Counts elements marked by a particular marker thread.
std::size_t countMarkedBy(const std::vector<int>& values, int markerId);

// Clears only the elements that belong to markerId.
void clearMarkedBy(std::vector<int>& values, int markerId);
