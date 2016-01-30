import glob
import os

from scripts.common import EXECUTABLE_NAME
from scripts.command import CommandSession

def build_go_sources(source_dir, output_dir, compiler_executable):
    go_source_files = glob.glob(os.path.join(source_dir, '*.go'))

    build_command = [
        compiler_executable,
        'build',
        '-o "' + os.path.join(output_dir, EXECUTABLE_NAME) + '"',
    ]
    os.environ['GOPATH'] = os.path.abspath('scripts/common/lang_go')
    build_command.extend(go_source_files)
    session = CommandSession()
    session.add_command(*build_command)
    session.run()

def build_go_sources_with_gccgo(source_dir, output_dir, compiler_executable):
    go_source_files = glob.glob(os.path.join(source_dir, '*.go'))

    build_command = [
        compiler_executable,
        'build',
        '-o "' + os.path.join(output_dir, EXECUTABLE_NAME) + '"',
        '-compiler gccgo',
        '-gccgoflags "-march=native -O3"',
    ]
    os.environ['GOPATH'] = os.path.abspath('scripts/common/lang_go')
    build_command.extend(go_source_files)
    session = CommandSession()
    session.add_command(*build_command)
    session.run()
