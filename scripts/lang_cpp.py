import os
from scripts.command import CommandSession

VC_VARS_BAT = r'C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat'

def prepare_vc_environment(session):
    session.add_command(VC_VARS_BAT, 'amd64')

def default_build(source_dir, output_dir):
    build_multiple_files(source_dir, output_dir)

def build_multiple_files(source_dir, output_dir):
    cpp_source_files = [os.path.join(source_dir, f) for f in os.listdir(source_dir) if f.endswith('.cpp')]

    build_command_prefix = [
        'cl', 
        '/Ox', 
        '/Ob1',
        '/Oi',
        '/Ot',
        '/GL',
        '/EHsc',
        '/c'
    ]

    session = CommandSession()
    prepare_vc_environment(session)

    obj_files = []

    for cpp_source_file in cpp_source_files:
        obj_file = os.path.splitext(os.path.basename(cpp_source_file))[0] + '.obj'
        obj_file = os.path.join(output_dir, obj_file)
        obj_files.append(obj_file)
        build_command = list(build_command_prefix)
        build_command.append('/Fo"' + obj_file + '"')
        build_command.append(cpp_source_file)
        session.add_command(*build_command)

    linker_command = [
        'link',
        '/OUT:"' + os.path.join(output_dir, 'benchmark.exe') + '"',
        '/LTCG',
        '/MACHINE:X64',
        '/INCREMENTAL:NO',
        '/SUBSYSTEM:CONSOLE',
        '/OPT:REF',
        '/OPT:ICF'
    ]
    linker_command.extend(obj_files)
    session.add_command(*linker_command)

    session.run()

def build_single_file(cpp_file, output_dir):
    build_command = [
        'cl', 
        '/Ox', 
        '/Ob1',
        '/Oi',
        '/Ot',
        '/GL',
        '/EHsc', 
        '/Fo"' + os.path.join(output_dir, 'benchmark.obj"'),
        '/Fe"' + os.path.join(output_dir, 'benchmark.exe"'),
        cpp_file
    ]
    session = CommandSession()
    prepare_vc_environment(session)
    session.add_command(*build_command)
    session.run()
