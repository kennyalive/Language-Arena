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
    int right = numbers.length - 1;
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

bool validateResult(int[] result, string answersFileName)
{
    auto answer = readNumbersFromFile(answersFileName);
    return result == answer;
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

    // return benchmark results
    int exitCode = elapsedTime;
    if (args.back == "validate")
    {
        string answersFileName = args[1] ~ "/sorted_random_numbers";
        exitCode = validateResult(array, answersFileName) ? 0 : -1;
    }
    return exitCode;
}
