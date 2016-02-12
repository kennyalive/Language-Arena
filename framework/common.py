import os

FRAMEWORK_PATH = os.path.dirname(os.path.abspath(__file__))
COMMON_DIR_PATH = os.path.join(FRAMEWORK_PATH, 'common')
PROJECT_ROOT_PATH = os.path.normpath(os.path.join(FRAMEWORK_PATH, '..'))

EXECUTABLE_NAME = 'benchmark.exe' if os.name == 'nt' else 'benchmark'
