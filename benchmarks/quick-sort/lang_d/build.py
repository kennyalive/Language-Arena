import os
import sys
from scripts.build_d import build_single_file_gdc

src = os.path.join(os.path.dirname(__file__), 'main.d')
out_dir = sys.argv[1]
build_single_file_gdc(src, out_dir)
