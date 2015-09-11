benchmark_builders = {
    'lang_cpp' : [
        {
            'name' : 'msvc',
            'builder': 'build_cpp_sources_with_msvc',
            'vcvars_path' : r'C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat'
        }
    ],

    'lang_d' : [
        {
            'name' : 'dmd',
            'builder' : 'build_d_sources_with_dmd',
            'path' : r'C:\D\dmd2\windows\bin'
        },

        {
            'name' : 'gdc',
            'builder' : 'build_d_sources_with_gdc',
            'path' : r'C:\D\gdc\bin'
        }
    ],

    'lang_go' : [
        {
            'name' : 'go',
            'builder' : 'build_go_sources',
            'path' :  r'C:\Go\bin'
        }
    ]
}
