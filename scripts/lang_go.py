import os
from scripts.command import CommandSession

def build_go_sources(source_dir, output_dir, builder_config):
    go_source_files = [os.path.join(source_dir, f) for f in os.listdir(source_dir) if f.endswith('.go')]
    build_command = [
        os.path.join(builder_config['path'], 'go'),
        'build',
        '-o "' + os.path.join(output_dir, 'benchmark.exe"')
    ]
    build_command.extend(go_source_files)
    session = CommandSession()
    session.add_command(*build_command)
    session.run()
