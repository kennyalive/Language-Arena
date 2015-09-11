import os
from scripts.command import CommandSession

def build_d_sources_with_dmd(source_dir, output_dir, builder_config):
    build_command = [
        os.path.join(builder_config['path'], 'rdmd'),
        '--build-only',
        '-O',
        '-inline',
        '-release',
        '-m64',
        '-boundscheck=off',
        '-od"' + output_dir + '"',
        '-of"' + os.path.join(output_dir, 'benchmark.exe"'),
        os.path.join(source_dir, 'main.d')
    ]
    session = CommandSession()
    session.add_command(*build_command)
    session.run()

def build_d_sources_with_gdc(source_dir, output_dir, builder_config):
    d_source_files = [os.path.join(source_dir, f) for f in os.listdir(source_dir) if f.endswith('.d')]
    build_command = [
        os.path.join(builder_config['path'], 'gdc'),
        '-O3',
        '-m64',
        '-fno-bounds-check',
        '-frelease',
        '-o"' + os.path.join(output_dir, 'benchmark.exe"')
    ]
    build_command.extend(d_source_files)
    session = CommandSession()
    session.add_command(*build_command)
    session.run()
