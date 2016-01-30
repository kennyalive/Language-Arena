import glob
import os
import sys

from scripts.common import EXECUTABLE_NAME
from scripts.command import CommandSession

def build_d_sources_with_dmd(source_dir, output_dir, compiler_executable):
    build_command = [
        compiler_executable,
        '--build-only',
        '-O',
        '-inline',
        '-release',
        '-m64',
        '-boundscheck=off',
        '-od"' + output_dir + '"',
        '-of"' + os.path.join(output_dir, EXECUTABLE_NAME) + '"',
        '-I' + 'scripts/common/lang_d',
        os.path.join(source_dir, 'main.d')
    ]
    session = CommandSession()
    session.add_command(*build_command)
    session.run()

def build_d_sources_with_gdc(source_dir, output_dir, compiler_executable):
    d_source_files = glob.glob(os.path.join(source_dir, '*.d'))
    d_source_files.append('scripts/common/lang_d/common.d')

    build_command = [
        compiler_executable,
        '-O3',
        '-m64',
        '-fno-bounds-check',
        '-frelease',
        '-o"' + os.path.join(output_dir, EXECUTABLE_NAME) + '"',
        '-I' + 'scripts/common/lang_d',
    ]
    build_command.extend(d_source_files)
    session = CommandSession()
    session.add_command(*build_command)
    session.run()

def build_d_sources_with_ldc(source_dir, output_dir, compiler_executable):
    d_source_files = glob.glob(os.path.join(source_dir, '*.d'))
    d_source_files.append('scripts/common/lang_d/common.d')

    build_command = [
        compiler_executable,
        '-O3',
        '-m64',
        '-release',
        '-od"' + output_dir + '"',
        '-of"' + os.path.join(output_dir, EXECUTABLE_NAME) + '"',
        '-I' + 'scripts/common/lang_d'
    ]
    build_command.extend(d_source_files)
    session = CommandSession()
    session.add_command(*build_command)
    session.run()
