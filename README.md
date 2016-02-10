DigitalWhip
===========

DigitalWhip is a performance benchmark of _statically typed programming languages that compile to native code_.

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
The compiler configuration is pretty simple. Just update paths in config.py located in the project's root.
Is is not required to configure all the compilers listed in config.py, the compiler would be skipped if it's not found.
Also refer to instructions in config.py file.

#### Starting the test
From the project's root run the launcher script. Depending on OS it's either

    ./run.sh
    
or

    run.bat

   


