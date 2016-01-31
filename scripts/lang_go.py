import glob
import os
import subprocess

from scripts.common import EXECUTABLE_NAME


def build_go_sources(source_dir, output_dir, compiler_executable):
    build_command = [
        compiler_executable,
        'build',
        '-o',
        os.path.join(output_dir, EXECUTABLE_NAME),
    ]
    go_source_files = glob.glob(os.path.join(source_dir, '*.go'))
    build_command.extend(go_source_files)
    os.environ['GOPATH'] = os.path.abspath('scripts/common/lang_go')
    subprocess.call(build_command)


def build_go_sources_with_gccgo(source_dir, output_dir, compiler_executable):
    build_command = [
        compiler_executable,
        'build',
        '-o',
        os.path.join(output_dir, EXECUTABLE_NAME),
        '-compiler gccgo',
        '-gccgoflags "-march=native -O3"',
    ]
    go_source_files = glob.glob(os.path.join(source_dir, '*.go'))
    build_command.extend(go_source_files)
    subprocess.call(build_command)
