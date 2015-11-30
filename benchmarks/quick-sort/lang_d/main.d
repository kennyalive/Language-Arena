import std.stdio;
import std.array;
import std.file;
import std.conv;
import std.path;
import std.string;
import std.algorithm;
import std.datetime;
import std.c.process;

void runtimeError(string message)
{
    writeln("runtime error: " ~ message);
    exit(-1);
}

void validationError(string message)
{
    writeln("validation error: " ~ message);
    exit(-2);
}

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

    return elapsedTime;
}
