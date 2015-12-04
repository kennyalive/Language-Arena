languages = [
    {
        'language' : 'lang_cpp',
        'display_name' : 'C++',
        'build_configurations' :
        [
            {
                'name' : 'msvc',
                'builder': 'build_cpp_sources_with_msvc',
                'vcvars_path' : r'C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat'
            }
        ]
    },
    {
        'language' : 'lang_d',
        'display_name' : 'D',
        'build_configurations' :
        [
            {
                'name' : 'dmd',
                'builder' : 'build_d_sources_with_dmd',
                'path' : r'C:\D\dmd2\windows\bin'
            },
            {
                'name' : 'gdc',
                'builder' : 'build_d_sources_with_gdc',
                'path' : r'C:\D\gdc\bin'
            },
            {
                'name' : 'ldc',
                'builder' : 'build_d_sources_with_ldc',
                'path' : r'c:\D\ldc\bin'
            }
        ]
    },
    {
        'language' : 'lang_go',
        'display_name' : 'Go',
        'build_configurations' :
        [
            {
                'name' : 'go',
                'builder' : 'build_go_sources',
                'path' :  r'C:\Go\bin'
            }
        ]
    }
]
