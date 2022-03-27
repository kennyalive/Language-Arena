import common
import glob
import os
import subprocess


def get_go_version(compiler_executable):
    return common.get_first_line_from_command_output([compiler_executable, 'version'])


def get_gccgo_version(compiler_executable):
    return common.get_first_line_from_command_output([compiler_executable, '--version'])


def build_go_sources(source_dir, output_dir, compiler_executable):
    build_command = [
        compiler_executable,
        'build',
        '-o',
        os.path.join(output_dir, common.EXECUTABLE_NAME),
    ]
    go_source_files = glob.glob(os.path.join(source_dir, '*.go'))
    build_command.extend(go_source_files)
    os.environ['GOPATH'] = os.path.join(common.COMMON_DIR_PATH, 'lang_go')
    os.environ['GO111MODULE'] = 'off'
    subprocess.call(build_command)


def build_go_sources_with_gccgo(source_dir, output_dir, compiler_executable):
    common_obj = os.path.join(output_dir, 'common.o')
    subprocess.call([
        compiler_executable,
        '-c',
        '-g',
        '-m64',
        '-O3',
        '-o',
        common_obj,
        os.path.join(common.COMMON_DIR_PATH, 'lang_go', 'src', 'common', 'common.go')
    ])

    main_obj = os.path.join(output_dir, 'main.o')
    build_command = [
        compiler_executable,
        '-c',
        '-g',
        '-m64',
        '-O3',
        '-o',
        main_obj,
        '-I' + output_dir,
    ]
    go_source_files = glob.glob(os.path.join(source_dir, '*.go'))
    build_command.extend(go_source_files)
    subprocess.call(build_command)

    subprocess.call([
        compiler_executable,
        '-o',
        os.path.join(output_dir, common.EXECUTABLE_NAME),
        common_obj,
        main_obj
    ])
