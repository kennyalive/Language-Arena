#include "../../../scripts/common.h"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <vector>

std::vector<int32_t> ReadNumbersFromFile(const std::string& fileName)
{
    std::ifstream file(fileName, std::ios_base::in | std::ios_base::binary);
    if (!file)
        RuntimeError("failed to open file " + fileName);

    int32_t numbersCount;
    file.read(reinterpret_cast<char*>(&numbersCount), 4);
    if (!file)
        RuntimeError("failed to read numbers count");

    std::vector<int32_t> numbers(numbersCount);
    file.read(reinterpret_cast<char*>(numbers.data()), numbersCount * 4);
    if (!file)
        RuntimeError("failed to read numbers data");
    return numbers;
}

template <typename T>
void QuickSort(T& container, int left, int right)
{
    int storeIndex = left;
    for (int i = left; i < right; i++)
    {
        if (container[i] <= container[right])
            std::swap(container[i], container[storeIndex++]);
    }
    std::swap(container[right], container[storeIndex]);

    if (left < storeIndex - 1)
        QuickSort(container, left, storeIndex - 1);
    if (storeIndex + 1 < right)
        QuickSort(container, storeIndex + 1, right);
}

int main(int argc, char* argv[])
{
    // prepare input data
    const auto fileName = JoinPath(argv[1], "random_numbers");
    auto array = ReadNumbersFromFile(fileName);

    // run benchmark
    Timer timer;
    QuickSort(array, 0, static_cast<int>(array.size()) - 1);
    int elapsedTime = static_cast<int>(timer.elapsed() * 1000);

    // validation
    if (array.size() != 4000000)
        ValidationError("invalid size");

    auto prevValue = array[0];
    for (size_t i = 1; i < array.size(); i++)
    {
        if (prevValue > array[i])
            ValidationError("array is not sorted");
        prevValue = array[i];
    }

    return elapsedTime;
}
