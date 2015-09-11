import os
from scripts.command import CommandSession

def build_go_sources(source_dir, output_dir, builder_config):
    build_command = [
        os.path.join(builder_config['path'], 'go'),
        'build',
        '-o "' + os.path.join(output_dir, 'benchmark.exe"'),
        os.path.join(source_dir, 'main.go')
    ]
    session = CommandSession()
    session.add_command(*build_command)
    session.run()
