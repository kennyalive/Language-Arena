import importlib
import os
import shutil
import subprocess
import sys
import time

from config import benchmark_builders

BUILD_DIR = 'build'
BENCHMARKS_DIR = 'benchmarks'
DATA_DIR = 'data'

def get_benchmark_languages(benchmark):
    directories = os.listdir(os.path.join(BENCHMARKS_DIR, benchmark))
    return sorted([dir for dir in directories if dir.startswith('lang_')])

def build_benchmark_with_builder(benchmark, language, builder_config):
    builder_func_name = builder_config['builder']
    builders_module = importlib.import_module('scripts.' + language)

    if builder_func_name not in dir(builders_module):
        print('failed to find builder function: ' + builder_func_name)
        sys.exit()

    output_dir_abs = os.path.abspath(os.path.join(BUILD_DIR, benchmark,
        language, builder_config['name']))
    language_dir = os.path.join(BENCHMARKS_DIR, benchmark, language)

    build_launcher_script = (
        "from scripts." + language +
        " import " + builder_func_name + "\n" +
        builder_func_name + "(r'" +
        language_dir + "', r'" +
        output_dir_abs + "'," +
        "eval(r'''" + repr(builder_config) + "'''))"
    )

    os.makedirs(output_dir_abs)
    subprocess.call(['python', '-c', build_launcher_script])

def build_benchmark(benchmark):
    os.environ['PYTHONPATH'] = os.path.dirname(os.path.realpath(__file__))
    for language in get_benchmark_languages(benchmark):
        language_builders = benchmark_builders.get(language)
        if language_builders is None:
            print('builders for ' + language +
                ' are not specified in config.py')
            continue
        for builder in language_builders:
            build_benchmark_with_builder(benchmark, language, builder)

def run_benchmark(benchmark):
    data_dir = os.path.join(BENCHMARKS_DIR, benchmark, DATA_DIR)
    for language in get_benchmark_languages(benchmark):
        for builder in benchmark_builders.get(language, []):
            print(language + '/' + builder['name'])
            executable = os.path.join(BUILD_DIR, benchmark, language,
                builder['name'], 'benchmark.exe')
            benchmark_result = subprocess.call([executable, data_dir])
            elapsed_time = benchmark_result / 1000.0
            print("{:.3f}".format(elapsed_time))

if __name__ == '__main__':
    if os.path.exists(BUILD_DIR):
        shutil.rmtree(BUILD_DIR)
    os.makedirs(BUILD_DIR)

    benchmarks = sorted(os.listdir('benchmarks'))

    if len(sys.argv) > 1:
        if sys.argv[1] in benchmarks:
            benchmarks = [sys.argv[1]]
        else:
            print('unknown benchmark ' + sys.argv[1])
            sys.exit()

    for benchmark in benchmarks:
        build_benchmark(benchmark)

    print("")
    for benchmark in benchmarks:
        print('------ Running ' + benchmark + ' ------')
        run_benchmark(benchmark)
