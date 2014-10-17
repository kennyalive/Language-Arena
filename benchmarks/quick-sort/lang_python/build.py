import os
import sys
import shutil

src = os.path.join(os.path.dirname(__file__), 'benchmark.py')
out_dir = sys.argv[1]
shutil.copy(src, out_dir)
