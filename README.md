DigitalWhip
===========

DigitalWhip is a performance benchmark of _statically typed programming languages that compile to native code_.

Like any other benchmark DigitalWhip does not tell the truth but it provides a perspective.

Story: once upon a time three guests were invited to the benchmark party: Mr. [**C++**](https://isocpp.org/), Mr. [**D**](https://dlang.org/) and Mr. [**Go**](https://golang.org/). The others were not invited because the master had a small house and also some of them were ugly. The guests were tested.

![Benchmark image](https://cloud.githubusercontent.com/assets/4964024/12995717/4130ed9c-d132-11e5-9b4a-6ef4291bc7bf.jpg)

<table>
    <tr>
        <td colspan="3" align="center"><h4>Example results 2016</h4></td>
    </tr>
    <tr>
        <td colspan="3">
         Linux<br/>
         <a href="https://github.com/artemalive/DigitalWhip/files/157778/example_linux_results.txt">Full report</a>
        </td>
    </tr>
    <tr>
    
        <td>
            <table>
                <tr>
                    <td><b>Simple benchmarks</b></td>
                </tr>
                <tr>
                    <td>
                       <table>
                         <tr><td>Language</td><td>Relative time</td></tr>
                         <tr><td>D</td><td>1.00</td></tr>
                         <tr><td>C++</td><td>1.06</td></tr>
                         <tr><td>Go</td><td>1.27</td></tr>
                       </table>
                    </td>
                </tr>
            </table>
        </td>
        
        <td>
            <table>
                <tr>
                    <td><b>Complex benchmarks</b></td>
                </tr>
                <tr>
                    <td>
                       <table>
                         <tr><td>Language</td><td>Relative time</td></tr>
                         <tr><td>C++</td><td>1.00</td></tr>
                         <tr><td>D</td><td>1.15</td></tr>
                         <tr><td>Go</td><td>3.04</td></tr>
                       </table>
                    </td>
                </tr>
            </table>
        </td>
        
        <td>
            <table>
                <tr>
                    <td><b>Summary</b></td>
                </tr>
                <tr>
                    <td>
                       <table>
                         <tr><td>Place</td><td>Language</td><td>Score</td></tr>
                         <tr><td>1</td><td>C++</td><td>50</td></tr>
                         <tr><td>2</td><td>D</td><td>40</td></tr>
                         <tr><td>3</td><td>Go</td><td>0</td></tr>
                       </table>
                    </td>
                </tr>
            </table>
        </td>
    </tr>
    
    <tr>
        <td colspan="3">
        Windows<br/>
        <a href="https://github.com/artemalive/DigitalWhip/files/157777/example_windows_results.txt">Full report</a>
        </td>
    </tr>
    <tr>
    
        <td>
            <table>
                <tr>
                    <td><b>Simple benchmarks</b></td>
                </tr>
                <tr>
                    <td>
                       <table>
                         <tr><td>Language</td><td>Relative time</td></tr>
                         <tr><td>D</td><td>1.00</td></tr>
                         <tr><td>C++</td><td>1.17</td></tr>
                         <tr><td>Go</td><td>1.38</td></tr>
                       </table>
                    </td>
                </tr>
            </table>
        </td>
        
        <td>
            <table>
                <tr>
                    <td><b>Complex benchmarks</b></td>
                </tr>
                <tr>
                    <td>
                       <table>
                         <tr><td>Language</td><td>Relative time</td></tr>
                         <tr><td>C++</td><td>1.00</td></tr>
                         <tr><td>D</td><td>1.03</td></tr>
                         <tr><td>Go</td><td>2.71</td></tr>
                       </table>
                    </td>
                </tr>
            </table>
        </td>
        
        <td>
            <table>
                <tr>
                    <td><b>Summary</b></td>
                </tr>
                <tr>
                    <td>
                       <table>
                         <tr><td>Place</td><td>Language</td><td>Score</td></tr>
                         <tr><td>1</td><td>C++, D</td><td>50</td></tr>
                         <tr><td>2</td><td>Go</td><td>0</td></tr>
                       </table>
                    </td>
                </tr>
            </table>
        </td>
    </tr>
</table>

Getting up and running
----------------------
#### 1. Check prerequisites
1. Local clone of DigitalWhip repository.
2. Python (2.7 or 3.x).
3. Actual compilers for benchmarking.

#### 2. Configure compilers
Update `config.py` from the project's root by specifying paths to the compilers you are interested in.

#### 3. Start the test
From the project's root run the launcher script. Depending on OS it's either

    ./run.sh
    
or

    run.bat
    
Benchmarks
----------
DigitalWhip divides benchmarks into two categories: simple benchmarks and complex benchmarks.

***

**Simple benchmarks**

![simple logo](https://cloud.githubusercontent.com/assets/4964024/12995843/57628dd6-d133-11e5-9e12-7a8cb5e1ccfd.jpg)

The goal of a simple benchmark is to perform basic sanity check. It is expected that compilers will produce executables with similar performance for simple benchmarks. If it's not the case then either compiler is not mature enough or architecture of programming language imposes limitations for generating high performance code even for simple scenarios.

***

**Complex benchmarks**

![complex logo](https://cloud.githubusercontent.com/assets/4964024/12994169/29e76414-d127-11e5-9e8a-c45d455f0cc1.jpg)

Complex benchmarks implement non-trivial algorithms. These benchmarks have higher chances to detect the influence of the language design on performance characteristics. They also provide an opportunity for the compilers to demonstrate their optimization skills.

***

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

Scoring is performed on a per-language basis. If the benchmark for particular language is compiled with multiple compilers then the fastest result is selected for scores calculation.

Maximum possible score = 10 + 10 + 20 + 20 = 60

Additionally per-language and per-compiler relative running times are computed.

Compiler notes
--------------
**Linux support**:
* C++: gcc, clang
* D: dmd, gdc, ldc
* Go: go, gccgo

_Linux specific information:_
* It takes a lot of time for gccgo to digest complex benchmarks. Be patient or just comment it out in config.py.

**Windows support**:
* C++: msvc
* D: dmd, gdc, ldc
* Go : go

_Windows specific information:_
* LDC D compiler requires installation of Visual Studio Community 2015 to link properly.
* GDC D compiler is not officially supported on Windows but prebuild binaries work fine with this benchmark: ftp://ftp.gdcproject.org/binaries/5.2.0/x86_64-w64-mingw32/gdc-5.2.0+2.066.1.7z

**Where to get compilers?**
* Visual Studio Community 2015: https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx
* Dlang compilers: http://dlang.org/download.html
* Golang: https://golang.org/dl/
