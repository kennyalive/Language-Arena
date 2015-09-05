benchmark_builders = {
    'cpp' : [
        {
            'name' : 'msvc',
            'builder': 'build_cpp_sources_with_msvc',
            'path' : r'C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat'
        }
    ],

    'd' : [
        {
            'name' : 'dmd',
            'builder' : 'build_d_sources_with_dmd',
            'path' : r'C:\D\dmd2\windows\bin\dmd.exe'
        },

        {
            'name' : 'gdc',
            'builder' : 'build_d_sources_with_gdc',
            'path' : r'C:\D\gdc\bin\gdc.exe'
        }
    ],

    'go' : [
        {
            'name' : 'go',
            'builder' : 'build_go_sources',
            'path' :  r'C:\Go\bin\go.exe'
        }
    ]
}
