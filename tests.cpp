#include "MarkerLogic.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
int failures = 0;
void check(bool condition, const std::string& name) {
    if (condition) std::cout << "[OK] " << name << '\n';
    else { std::cout << "[FAIL] " << name << '\n'; ++failures; }
}
}

int main() {
    check(indexFromRandom(17, 5) == 2 && indexFromRandom(4, 5) == 4,
          "indexFromRandom returns index modulo array size");

    const std::vector<int> sample{0, 1, 2, 1, 3, 1, 0};
    check(countMarkedBy(sample, 1) == 3 && countMarkedBy(sample, 2) == 1,
          "countMarkedBy counts only matching elements");

    std::vector<int> values{0, 2, 1, 1, 3, 2, 0};
    clearMarkedBy(values, 1);
    check(values == std::vector<int>({0, 2, 0, 0, 3, 2, 0}),
          "clearMarkedBy clears only the selected marker");

    bool threw = false;
    try { (void)indexFromRandom(10, 0); }
    catch (const std::invalid_argument&) { threw = true; }
    check(threw, "indexFromRandom rejects an empty array");

    std::cout << (failures == 0 ? "All tests passed.\n" : "Some tests failed.\n");
    return failures == 0 ? 0 : 1;
}
