import os
import shutil

def default_build(source_dir, output_dir):
    python_file = os.path.join(source_dir, 'main.py')
    shutil.copy(python_file, os.path.join(output_dir, 'benchmark.py'))
