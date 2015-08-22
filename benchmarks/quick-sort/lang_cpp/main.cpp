#include "../../../scripts/Timer.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

std::vector<int32_t> ReadNumbersFromFile(const std::string& fileName)
{
    std::ifstream file(fileName, std::ios_base::in | std::ios_base::binary);

    int32_t numbersCount;
    file.read(reinterpret_cast<char*>(&numbersCount), 4);

    std::vector<int32_t> numbers(numbersCount);
    file.read(reinterpret_cast<char*>(numbers.data()), numbersCount * 4);

    return numbers;
}

template <typename T>
void QuickSort(T& container, int left, int right)
{
    if (left >= right)
        return;

    int storeIndex = left;
    for (int i = left; i < right; i++)
    {
        if (container[i] <= container[right])
            std::swap(container[i], container[storeIndex++]);
    }
    std::swap(container[right], container[storeIndex]);

    QuickSort(container, left, storeIndex - 1);
    QuickSort(container, storeIndex + 1, right);
}

int main(int argc, char* argv[])
{
    // prepare input data
    const auto fileName = std::string(argv[1]) + "/random_numbers";
    auto array = ReadNumbersFromFile(fileName);

    // run benchmark
    Timer timer;
    QuickSort(array, 0, static_cast<int>(array.size()) - 1);
    return timer.elapsed() * 1000;
}
