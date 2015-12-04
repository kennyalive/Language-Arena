import std.stdio;
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
