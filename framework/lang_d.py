import common
import glob
import os
import subprocess


def build_d_sources_with_dmd(source_dir, output_dir, compiler_executable):
    subprocess.call([
        compiler_executable,
        '--build-only',
        '-O',
        '-inline',
        '-release',
        '-m64',
        '-boundscheck=off',
        '-od' + output_dir,
        '-of' + os.path.join(output_dir, common.EXECUTABLE_NAME),
        '-I' + os.path.join(common.FRAMEWORK_PATH, 'common/lang_d'),
        os.path.join(source_dir, 'main.d')
    ])


def build_d_sources_with_gdc(source_dir, output_dir, compiler_executable):
    build_command = [
        compiler_executable,
        '-O3',
        '-m64',
        '-fno-bounds-check',
        '-frelease',
        '-o' + os.path.join(output_dir, common.EXECUTABLE_NAME),
        '-I' + os.path.join(common.FRAMEWORK_PATH, 'common/lang_d'),
    ]
    d_source_files = glob.glob(os.path.join(source_dir, '*.d'))
    d_source_files.append(os.path.join(common.FRAMEWORK_PATH, 'common/lang_d/common.d'))
    build_command.extend(d_source_files)
    subprocess.call(build_command)


def build_d_sources_with_ldc(source_dir, output_dir, compiler_executable):
    build_command = [
        compiler_executable,
        '-O3',
        '-m64',
        '-release',
        '-od' + output_dir,
        '-of' + os.path.join(output_dir, common.EXECUTABLE_NAME),
        '-I' + os.path.join(common.FRAMEWORK_PATH, 'common/lang_d')
    ]
    d_source_files = glob.glob(os.path.join(source_dir, '*.d'))
    d_source_files.append(os.path.join(common.FRAMEWORK_PATH, 'common/lang_d/common.d'))
    build_command.extend(d_source_files)
    subprocess.call(build_command)
