DigitalWhip
===========

DigitalWhip is a performance benchmark of _statically typed programming languages that compile to native code_.

DigitalWhip does not tell the truth but it provides a perspective. Objects appear different in size depending on your viewpoint.

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
Update `config.py` from the project's root by specifying paths to the compilers you are interested in. It is not required to configure all the compilers listed in `config.py`, the missing compilers would be ignored.

#### Starting the test
From the project's root run the launcher script. Depending on OS it's either

    ./run.sh
    
or

    run.bat
    
Benchmarks
----------
DigitalWhip divides benchmarks into two categories: simple benchmarks and complex benchmarks.

##### Simple benchmarks
Simple benchmarks implement relatively simple algorithms. It is expected that compilers will produce executables with similar performance for simple benchmarks. If it's not the case it means that either compiler is not mature enough or architecture of programming language itself imposes limitations for generating high performance code.

##### Complex benchmarks
Complex benchmarks implement non-trivial algorithms. These kind of benchmarks have higher changes to detect the influence of the language design on performance characteristics and they also provide an opportunity for the compilers to demonstrate their optimization skills.

DigitalWhip provides 2 simple benchmarks and 2 complex benchmarks:

| Benchmark             | Type    |
| --------------------- |-------- |
| quick-sort            | simple  |
| reflections-math      | simple  |
| kdtree-construction   | complex |
| kdtree-raycast        | complex |


Scoring
-------

| Benchmark Type | 1st place  | 2nd place  | >= 3rd place |
| -------------- |------------|------------|--------------|
| simple         | 10 points  | 5  points  | 0 points     |
| complex        | 20 points  | 10 points  | 0 points     |

Scoring is performed on the language level not on the compiler level. If the benchmark is compiled with multiple compilers then the fastest result is selected for scores calculation.

Maximum possible language score is 10 + 10 + 20 + 20 = 60

Additionally per-language and per-compiler related running times are computed. The fastest time is considered as a unity and other times are scaled proportionally.

Compiler notes
--------------
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


