import std.stdio;
import std.array;
import std.file;
import std.conv;
import std.string;
import std.algorithm;
import std.datetime;

int[] readNumbersFromFile(string fileName)
{
    auto file = File(fileName);
    auto a = appender!(int[]);
    foreach(string line; lines(file))
        a.put(to!int(strip(line)));
    return a.data;
}

void quickSort(int[] numbers)
{
    auto right = numbers.length - 1;
    int medianValue = numbers[right];
    int storeIndex = 0;
    foreach (i, value; numbers[0..$-1])
    {
        if (value <= medianValue)
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
    string fileName = args[1] ~ "/random_numbers";
    auto array = readNumbersFromFile(fileName);

    // run benchmark
    StopWatch sw;
    sw.start();
    quickSort(array);
    sw.stop();
    int elapsedTime = to!int(sw.peek().msecs());

    //validate results
    version(assert)
    {
        foreach (i, number; array)
            if (i != array.length - 1)
                assert(array[i] < array[i + 1]);
    }

    // return benchmark results
    int exitCode = elapsedTime;
    return exitCode;
}
