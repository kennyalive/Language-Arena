DigitalWhip
===========

DigitalWhip is a performance benchmark of _statically typed programming languages that compile to native code_.

![cpp_logo](https://cloud.githubusercontent.com/assets/4964024/23078950/673511b2-f553-11e6-8fde-53b318ae2a18.png)
![dlang_logo](https://cloud.githubusercontent.com/assets/4964024/23079892/741f50fa-f557-11e6-8815-e15ad5e48b7e.png)
![go_logo](https://cloud.githubusercontent.com/assets/4964024/23079852/49f3268a-f557-11e6-8af3-78cac1b699ce.png)

Example results 2015
--------------------
<table>
    <tr>
        <td colspan="3">
         Linux (<a href="https://github.com/artemalive/DigitalWhip/files/157778/example_linux_results.txt">full report</a>)
        </td>
    </tr>
    <tr>
        <td>
            <table>
                <tr><td colspan=2><b>Simple benchmarks</b></td></tr>
                <tr><td>Language</td><td>Time</td></tr>
                <tr><td>D</td><td>1.00</td></tr>
                <tr><td>C++</td><td>1.06</td></tr>
                <tr><td>Go</td><td>1.27</td></tr>
            </table>
        </td>
        <td>
            <table>
                <tr><td colspan=2><b>Complex benchmarks</b></td></tr>
                <tr><td>Language</td><td>Time</td></tr>
                <tr><td>C++</td><td>1.00</td></tr>
                <tr><td>D</td><td>1.15</td></tr>
                <tr><td>Go</td><td>3.04</td></tr>
            </table>
        </td>
        <td>
            <table>
                <tr><td colspan=3><b>Summary</b></td></tr>
                <tr><td>Place</td><td>Language</td><td>Score</td></tr>
                <tr><td>1</td><td>C++</td><td>50</td></tr>
                <tr><td>2</td><td>D</td><td>40</td></tr>
                <tr><td>3</td><td>Go</td><td>0</td></tr>
            </table>
        </td>
    </tr>
    
    <tr>
        <td colspan="3">
        Windows (<a href="https://github.com/artemalive/DigitalWhip/files/157777/example_windows_results.txt">full report</a>)
        </td>
    </tr>
    <tr>
        <td>
            <table>
                <tr><td colspan=2><b>Simple benchmarks</b></td></tr>
                <tr><td>Language</td><td>Time</td></tr>
                <tr><td>D</td><td>1.00</td></tr>
                <tr><td>C++</td><td>1.17</td></tr>
                <tr><td>Go</td><td>1.38</td></tr>
            </table>
        </td>
        <td>
            <table>
                <tr><td colspan=2><b>Complex benchmarks</b></td></tr>
                <tr><td>Language</td><td>Time</td></tr>
                <tr><td>C++</td><td>1.00</td></tr>
                <tr><td>D</td><td>1.03</td></tr>
                <tr><td>Go</td><td>2.71</td></tr>
            </table>
        </td>
        <td>
            <table>
                <tr><td colspan=3><b>Summary</b></td></tr>
                <tr><td>Place</td><td>Language</td><td>Score</td></tr>
                <tr><td>1</td><td>C++, D</td><td>50</td></tr>
                <tr><td>2</td><td>Go</td><td>0</td></tr>
            </table>
        </td>
    </tr>
</table>

Getting up and running
----------------------
Python is required to run the benchmark.
Update `config.py` from the project's root by specifying paths to the compilers you are interested in.
Run the launcher script (_./run.sh_ or _run.bat_)

Benchmarks
----------

**Simple benchmarks** (quick-sort, reflections-math)

The goal of a simple benchmark is to perform basic sanity check. It is expected that compilers will produce executables with similar performance for simple benchmarks. If it's not the case then either compiler is not mature enough or architecture of programming language imposes limitations for generating high performance code even for simple scenarios.

**Complex benchmarks** (kdtree-construction, kdtree-raycast)

Complex benchmarks implement non-trivial algorithms. These benchmarks have higher chances to detect the influence of the language design on performance characteristics. They also provide an opportunity for the compilers to demonstrate their optimization skills.

Scoring
-------

| Benchmark Type | 1st place  | 2nd place  | >= 3rd place |
| -------------- |------------|------------|--------------|
| simple         | 10 points  | 5  points  | 0 points     |
| complex        | 20 points  | 10 points  | 0 points     |

Scoring is performed on a per-language basis. If the benchmark for particular language is compiled with multiple compilers then the fastest result is selected for scores calculation.

Maximum possible score = 10 + 10 + 20 + 20 = 60

Additionally per-language and per-compiler relative running times are computed.

Compiler notes
--------------
**Linux support**:
* C++: gcc, clang
* D: dmd, gdc, ldc
* Go: go, gccgo

**Windows support**:
* C++: msvc
* D: dmd, gdc, ldc
* Go : go

**Where to get compilers?**
* Visual Studio Community 2015: https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx
* Dlang compilers: http://dlang.org/download.html
* Golang: https://golang.org/dl/
