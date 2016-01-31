import glob
import os
import subprocess

from scripts.common import EXECUTABLE_NAME


def build_cpp_sources_with_msvc(source_dir, output_dir, vcvars_path):
    build_cpp_prefix = [
        'cl',
        '/c',
        '/O2',
        '/GL',
        '/EHsc',
        '/nologo',
        '/D "NDEBUG"',
        '/I scripts/common/lang_cpp'
    ]
    build_command = [
        '"' + vcvars_path + '"',
        'amd64',
    ]
    obj_files = []

    cpp_source_files = glob.glob(os.path.join(source_dir, '*.cpp'))
    for cpp_source_file in cpp_source_files:
        obj_file = os.path.splitext(os.path.basename(cpp_source_file))[0] + '.obj'
        obj_file = os.path.join(output_dir, obj_file)
        obj_files.append(obj_file)

        build_cpp_command = list(build_cpp_prefix)
        build_cpp_command.append('/Fo"' + obj_file + '"')
        build_cpp_command.append(cpp_source_file)

        build_command.append('&')
        build_command.extend(build_cpp_command)

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
    build_command.append('&')
    build_command.extend(linker_command)

    subprocess.call(' '.join(build_command), shell=True)


def build_cpp_sources_with_gcc(source_dir, output_dir, executable_path):
    build_command = [
        executable_path,
        '-std=c++11',
        '-m64',
        '-O3',
        '-s',
        '-o',
        os.path.join(output_dir, EXECUTABLE_NAME),
        '-Iscripts/common/lang_cpp'
    ]
    cpp_source_files = glob.glob(os.path.join(source_dir, '*.cpp'))
    build_command.extend(cpp_source_files)
    subprocess.call(build_command)
