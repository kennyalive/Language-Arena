import std.conv;
import std.file;
import std.string;
import std.stdio;
import core.stdc.stdlib;

void runtimeError(string message)
{
    writeln("runtime error: " ~ message);
    exit(1);
}

void validationError(string message)
{
    writeln("validation error: " ~ message);
    exit(2);
}

void storeBenchmarkTiming(string path, int time)
{
    try
    {
        std.file.write(path, to!string(time));
    }
    catch (FileException)
    {
        runtimeError("failed to store benchmark timing");
    }
}

void assertEquals(T)(T actual, T expected, string message)
{
    if (actual != expected)
    {
        validationError(format("%s\nactual value %s, expected value %s",
            message, actual, expected));
    }
}

void assertEqualsHex(ulong actual, ulong expected, string message)
{
    if (actual != expected)
    {
        validationError(format("%s\nactual value %x, expected value %x",
            message, actual, expected));
    }
}

pure nothrow @nogc
ulong combineHashes(ulong hash1, ulong hash2)
{
    return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
}
