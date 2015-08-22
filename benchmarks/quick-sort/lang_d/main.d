import std.stdio;
import std.array;
import std.file;
import std.conv;
import std.string;
import std.algorithm;
import std.datetime;

int[] readNumbersFromFile(string fileName)
{
    auto file = File(fileName, "rb");

    int[1] numbersCount;
    file.rawRead(numbersCount);

    int[] numbers;
    numbers.length = numbersCount[0];
    file.rawRead(numbers);

    return numbers;
}

void quickSort(int[] numbers)
{
    auto right = numbers.length - 1;

    int storeIndex = 0;
    foreach (i, value; numbers[0..$-1])
    {
        if (value <= numbers[right])
            swap(numbers[i], numbers[storeIndex++]);
    }
    swap(numbers[right], numbers[storeIndex]);

    if (storeIndex > 0)
        quickSort(numbers[0..storeIndex]);
    if (right > storeIndex)
        quickSort(numbers[storeIndex+1..$]);
}

int main(string[] args)
{
    // prepare input data
    const auto fileName = args[1] ~ "/random_numbers";
    auto array = readNumbersFromFile(fileName);

    // run benchmark
    StopWatch sw;
    sw.start();
    quickSort(array);
    return to!int(sw.peek().msecs());
}
