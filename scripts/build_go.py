import os
from scripts.command import CommandSession

def build_single_file(go_file, output_dir):
    build_command = [
        'go', 
        'build', 
        '-o "' + os.path.join(output_dir, 'benchmark.exe"'), 
        go_file
    ]
    session = CommandSession()
    session.add_command(*build_command)
    session.run()
