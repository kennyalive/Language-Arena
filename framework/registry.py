languages = [
    {
        'language' : 'lang_cpp',
        'display_name' : 'C++',
        'build_configurations' :
        [
            {
                'compiler': 'msvc',
                'builder': 'build_cpp_sources_with_msvc',
            },
            {
                'compiler': 'gcc',
                'builder': 'build_cpp_sources_with_gcc',
            },
            {
                'compiler': 'clang',
                'builder': 'build_cpp_sources_with_clang',
            }
        ]
    },
    {
        'language' : 'lang_d',
        'display_name' : 'D',
        'build_configurations' :
        [
            {
                'compiler' : 'dmd',
                'builder' : 'build_d_sources_with_dmd',
            },
            {
                'compiler' : 'gdc',
                'builder' : 'build_d_sources_with_gdc',
            },
            {
                'compiler' : 'ldc',
                'builder' : 'build_d_sources_with_ldc',
            }
        ]
    },
    {
        'language' : 'lang_go',
        'display_name' : 'Go',
        'build_configurations' :
        [
            {
                'compiler' : 'go',
                'builder' : 'build_go_sources',
            },
            {
                'compiler' : 'gccgo',
                'builder' : 'build_go_sources_with_gccgo',
            }
        ]
    }
]
