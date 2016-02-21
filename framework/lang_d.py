import common
import glob
import os
import subprocess


def get_dmd_version(rdmd_path):
    dmd_executable_name = 'dmd.exe' if os.name == 'nt' else 'dmd'
    dmd_path = os.path.join(os.path.dirname(rdmd_path), dmd_executable_name)
    return common.get_first_line_from_command_output([dmd_path, '--version'])


def get_gdc_version(compiler_executable):
    return common.get_first_line_from_command_output([compiler_executable, '--version'])


def get_ldc_version(compiler_executable):
    lines = common.get_command_output([compiler_executable, '--version'])
    return ' '.join(map(str.strip, lines[:2]))


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
        '-I' + os.path.join(common.COMMON_DIR_PATH, 'lang_d'),
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
        '-I' + os.path.join(common.COMMON_DIR_PATH, 'lang_d'),
    ]
    d_source_files = glob.glob(os.path.join(source_dir, '*.d'))
    d_source_files.append(os.path.join(common.COMMON_DIR_PATH, 'lang_d', 'common.d'))
    build_command.extend(d_source_files)
    subprocess.call(build_command)


def build_d_sources_with_ldc(source_dir, output_dir, compiler_executable):
    build_command = [
        compiler_executable,
        '-O3',
        '-m64',
        '-release',
        '-boundscheck=off',
        '-singleobj',
        '-od' + output_dir,
        '-of' + os.path.join(output_dir, common.EXECUTABLE_NAME),
        '-I' + os.path.join(common.COMMON_DIR_PATH, 'lang_d')
    ]
    d_source_files = glob.glob(os.path.join(source_dir, '*.d'))
    d_source_files.append(os.path.join(common.COMMON_DIR_PATH, 'lang_d', 'common.d'))
    build_command.extend(d_source_files)
    subprocess.call(build_command)
