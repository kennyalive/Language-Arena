import os
import subprocess

FRAMEWORK_PATH = os.path.dirname(os.path.abspath(__file__))
COMMON_DIR_PATH = os.path.join(FRAMEWORK_PATH, 'common')
PROJECT_ROOT_PATH = os.path.normpath(os.path.join(FRAMEWORK_PATH, '..'))

EXECUTABLE_NAME = 'benchmark.exe' if os.name == 'nt' else 'benchmark'


def get_command_output(args):
    process = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
    return process.communicate()[0].splitlines()


def get_first_line_from_command_output(args):
    lines = get_command_output(args)
    return lines[0] if len(lines) > 0 else ''
