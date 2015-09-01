configs = [
    {
        'lang' : 'cpp',
        'build_flavours' : [{
            'name' : 'msvc2015',
            'vc_vars' : r'C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat'
        }]
    },
    {
        'lang' : 'd',
        'build_flavours' : [ {
            'name' : 'gdc',
            'compiler' : r'C:\\D\\gdc\\bin\\gdc.exe'
        }]
    },
    {
        'lang' : 'go',
        'build_flavours' : [{
            'name' : 'go',
            'compiler' : r'C:\\Go\\bin\\go.exe'
        }]
    }
]
