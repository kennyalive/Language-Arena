DigitalWhip
===========

DigitalWhip is a performance benchmark of _statically typed programming languages that compile to native code_.

DigitalWhip provides only a single perspective on the subject of performance. Objects appear different in size depending on your viewpoint.

Three guests were invited to the benchmark party: Mr. [C++](https://isocpp.org/), Mr. [D](https://dlang.org/) and Mr. [Go](https://golang.org/).
The others were not invited because the master had a small house and also some of them were ugly.

![Benchmark image](https://github.com/artemalive/DigitalWhip/raw/master/whip.jpg)

Getting up and running
----------------------
#### Prerequisites
1. Local clone of DigitalWhip repository.
2. Python (2.7 or 3.x).
3. Actual compilers for benchmarking.

#### Compilers configuration
The compiler configuration is pretty simple. Just update the paths in config.py located in the project's root.
Is is not required to configure all the compilers listed in config.py, the compiler would be skipped if it's not found.

#### Starting the test
From the project's root run the launcher script. Depending on OS it's either

    ./run.sh
    
or

    run.bat
    
Benchmarks
----------
DigitalWhip divides benchmarks into 2 categories: simple benchmarks and complex benchmarks.

##### Simple benchmarks
The simple benchmarks implement some basic algorithm with simple logic and small amount of code. It is expected that compilers will produce executables with similar performance for simple benchmarks. If it's not the case it means that either compiler is not mature enough or architecture of programming language itself imposes limitations for generating high performance code.

##### Complex benchmarks
The complex benchmarks implement non-trivial algorithms. This allows to test influence of language design on performance characteristics and it also provides an opportunity for the compilers to demonstrate theirs optimization capabilities.

DigitalWhip provides 2 simple benchmarks and 2 complex benchmarks:

| Benchmark             | Type    |
| --------------------- |-------- |
| quick-sort            | simple  |
| reflections-math      | simple  |
| kdtree-construction   | complex |
| kdtree-raycast        | complex |


Scoring
-------

| Benchmark Type | 1st place points | 2nd place points | >= 3rd place points |
| -------------- |------------------|------------------|---------------------|
| simple         | 10               | 5                | 0                   |
| complex        | 20               | 10               | 0                   |

Scoring is performed on the language level not on the compiler level. If the benchmark is compiled with multiple compilers then the fastest result is selected for scores computation.

Maximum possible language score is 10 + 10 + 20 + 20 = 60

In addition to per-language scores the per-language and per-compiler related running times are computed. The fastest time is considered as a unity and other times are scaled proportionally.

Compiler notes
--------------
As mentioned above the compilers can be configured by modifying config.py.

Where to get compilers?
* Visual Studio Community 2015: https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx
* GCC is provided as part of the Linux distribution.
* Dlang compilers: http://dlang.org/download.html
* Golang: https://golang.org/dl/

Windows specific information:
* LDC D compiler required installation of Visual Studio 2015 to link properly.
* GDC D compiler is not officially supported on Windows but you can still download unsupported binaries from official site and they work fine for purposes of this benchmark.

Linux specific information:
* There were issues with clang-3.5 provided with Ubuntu 14.04. It was fixed by installing clang-3.6.


