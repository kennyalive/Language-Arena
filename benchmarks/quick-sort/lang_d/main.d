import std.algorithm;
import std.array;
import std.exception;
import std.file;
import std.conv;
import std.path;
import std.stdio;
import std.string;
import std.datetime;
import common;

int[] readNumbersFromFile(string fileName)
{
    try
    {
        auto file = File(fileName, "rb");
        int[1] numbersCount;
        file.rawRead(numbersCount);
        int[] numbers;
        numbers.length = numbersCount[0];
        file.rawRead(numbers);
        return numbers;
    }
    catch (ErrnoException e)
    {
        runtimeError("failed to read file content: " ~ fileName);
        assert(false);
    }
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

    if (storeIndex > 1)
        quickSort(numbers[0..storeIndex]);
    if (right - storeIndex > 1)
        quickSort(numbers[storeIndex+1..$]);
}

int main(string[] args)
{
    // prepare input data
    const auto fileName = buildPath(args[1], "random_numbers");
    auto array = readNumbersFromFile(fileName);

    // run benchmark
    StopWatch sw;
    sw.start();
    quickSort(array);
    auto elapsedTime = to!int(sw.peek().msecs());
    storeBenchmarkTiming(elapsedTime);

    // validation
    if (array.length != 4_000_000)
        validationError("invalid size");

    auto prevValue = array[0];
    foreach (value; array[1..$])
    {
        if (prevValue > value)
            validationError("array is not sorted");
        prevValue = value;
    }

    return 0;
}
