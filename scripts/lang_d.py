import os
from scripts.command import CommandSession

def build_benchmark_source(source_dir, output_dir):
    build_using_gdc(source_dir, output_dir)

def build_using_dmd(d_main_file, output_dir):
    build_command = [
        'rdmd', 
        '--build-only',
        '-O',
        '-inline',
        '-release',
        '-m64',
        '-boundscheck=off',
        '-od"' + output_dir + '"',
        '-of"' + os.path.join(output_dir, 'benchmark.exe"'),
        d_main_file
    ]
    session = CommandSession()
    session.add_command(*build_command)
    session.run()

def build_using_gdc(source_dir, output_dir):
    d_source_files = [os.path.join(source_dir, f) for f in os.listdir(source_dir) if f.endswith('.d')]
    build_command = [
        'gdc',
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
