import os
from scripts.command import CommandSession

VC_VARS_BAT = r'C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat'

def prepare_vc_environment(session):
    session.add_command(VC_VARS_BAT, 'amd64')

def build_single_file(cpp_file, output_dir):
    build_command = [
        'cl', 
        '/O2', 
        '/EHsc', 
        '/Fo"' + os.path.join(output_dir, 'benchmark.obj"'),
        '/Fe"' + os.path.join(output_dir, 'benchmark.exe"'),
        cpp_file
    ]
    session = CommandSession()
    prepare_vc_environment(session)
    session.add_command(*build_command)
    session.run()
