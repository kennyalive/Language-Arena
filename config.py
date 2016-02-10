# Please update these paths according to your environment.
#
# It is not required to configure all compilers here, 
# any subset will work (even empty).
# It is okay to leave configurations for missing compilers as it is,
# they will be skipped by the benchamrk.
#
# On Linux the following compilers are supported:
# C++: gcc, clang
# D: dmd, gdc, ldc
# Go: go, gccgo
#
# On Windows the following compilers are supported:
# C++: msvc,
# D: dmd, gdc, ldc
# Go : go

compilers = {
    # example of compilers configuration for Linux
    'gcc'   : r'/usr/bin/g++',
    'clang' : r'/usr/bin/clang++-3.6',
    'dmd'   : r'/usr/bin/rdmd',
    'gdc'   : r'/usr/local/gdc/bin/gdc',
    'ldc'   : r'/usr/local/ldc/bin/ldc2',
    'go'    : r'/usr/local/go/bin/go',
    'gccgo' : r'/usr/bin/gccgo',

    # example of compilers configuration for Windows
    #'msvc'  : r'C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat',
    #'dmd'   : r'C:\D\dmd2\windows\bin\rdmd.exe',
    #'gdc'   : r'C:\D\gdc\bin\gdc.exe',
    #'ldc'   : r'C:\D\ldc\bin\ldc2.exe',
    #'go'    : r'C:\Go\bin\go.exe',
}
