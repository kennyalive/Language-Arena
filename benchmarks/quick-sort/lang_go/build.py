import os
import sys
from scripts.build_go import build_single_file

src = os.path.join(os.path.dirname(__file__), 'main.go')
out_dir = sys.argv[1]
build_single_file(src, out_dir)
