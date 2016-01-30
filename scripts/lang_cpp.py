import glob
import os

from scripts.common import EXECUTABLE_NAME
from scripts.command import CommandSession

def build_cpp_sources_with_msvc(source_dir, output_dir, vcvars_path):
    cpp_source_files = glob.glob(os.path.join(source_dir, '*.cpp'))

    build_command_prefix = [
        'cl',
        '/c',
        '/O2',
        '/GL',
        '/EHsc',
        '/nologo',
        '/D "NDEBUG"',
        '/I scripts/common/lang_cpp'
    ]

    session = CommandSession()
    session.add_command(vcvars_path, 'amd64')

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
        '/OUT:"' + os.path.join(output_dir, EXECUTABLE_NAME) + '"',
        '/LTCG',
        '/OPT:REF',
        '/OPT:ICF',
        '/INCREMENTAL:NO',
        '/NOLOGO'
    ]
    linker_command.extend(obj_files)
    session.add_command(*linker_command)

    session.run()

def build_cpp_sources_with_gcc(source_dir, output_dir, executable_path):
    cpp_source_files = glob.glob(os.path.join(source_dir, '*.cpp'))

    build_command = [
        executable_path,
        '-std=c++11',
        '-m64',
        '-O3',
        '-s',
        '-o ' + os.path.join(output_dir, EXECUTABLE_NAME),
        '-Iscripts/common/lang_cpp'
    ]

    build_command.extend(cpp_source_files)

    session = CommandSession()
    session.add_command(*build_command)
    session.run()
