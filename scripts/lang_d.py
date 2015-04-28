import os
from scripts.command import CommandSession

def default_build(source_dir, output_dir):
    d_file = os.path.join(source_dir, 'main.d')
    build_single_file_dmd(d_file, output_dir)

def build_single_file_dmd(d_file, output_dir):
    build_command = [
        'dmd', 
        d_file,
        '-O',
        '-od"' + output_dir + '"',
        '-of"' + os.path.join(output_dir, 'benchmark.exe"')
    ]
    session = CommandSession()
    session.add_command(*build_command)
    session.run()

def build_single_file_gdc(d_file, output_dir):
    build_command = [
        'gdc',
        '-O3',
	    '-o"' + os.path.join(output_dir, 'benchmark.exe"'),
        d_file,
    ]
    session = CommandSession()
    session.add_command(*build_command)
    session.run()
