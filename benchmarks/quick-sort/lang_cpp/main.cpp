#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>

#include "../../../scripts/Timer.h"

std::vector<int> ReadNumbersFromFile(const std::string& file_name) {
    std::vector<int> numbers;
    std::ifstream file(file_name);
    if (file) {
        std::string line;
        while (std::getline(file, line)) {
            numbers.push_back(std::stoi(line));
        }
    }
    return numbers;
}

template <typename T>
void QuickSort(T& container, int left, int right) {
    if (left >= right) {
        return;
    }
    int store_index = left;
    for (int i = left; i < right; i++) {
        if (container[i] <= container[right]) {
            std::swap(container[i], container[store_index++]);
        }
    }
    std::swap(container[right], container[store_index]);
    QuickSort(container, left, store_index - 1);
    QuickSort(container, store_index + 1, right);
}

int main(int argc, char* argv[]) {
    // prepare input data
    const std::string file_name = std::string(argv[1]) + "/random_numbers";
    std::vector<int> array = ReadNumbersFromFile(file_name);

    // run benchmark
    Timer timer;
    QuickSort(array, 0, int(array.size()) - 1);
    const int elapsed_time = timer.elapsed() * 1000;

    // return benchmark results
    return elapsed_time;
}
