import os

FRAMEWORK_PATH = os.path.dirname(os.path.abspath(__file__))
EXECUTABLE_NAME = 'benchmark.exe' if os.name == 'nt' else 'benchmark'
