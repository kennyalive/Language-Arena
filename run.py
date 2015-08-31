import importlib
import os
import shutil
import subprocess
import sys
import time

BUILD_DIR = 'build'
BENCHMARKS_DIR = 'benchmarks'
DATA_DIR = 'data'

def get_benchmark_languages(benchmark):
    directories = os.listdir(os.path.join(BENCHMARKS_DIR, benchmark))
    return sorted([dir for dir in directories if dir.startswith('lang_')])

def build_benchmark(benchmark):
    # create build configuration for each available language
    build_config = []
    for language in get_benchmark_languages(benchmark):
        language_dir = os.path.join(BENCHMARKS_DIR, benchmark, language)
        output_dir   = os.path.abspath(os.path.join(BUILD_DIR, benchmark, language))

        language_script_module = importlib.import_module('scripts.' + language)
        if 'build_benchmark_source' in dir(language_script_module):
            os.makedirs(output_dir)
            build_config.append({
                'build_script': 'scripts.' + language,
                'output_dir': output_dir,
                'source_dir': language_dir
                })

    # do build for all languages
    os.environ['PYTHONPATH'] = os.path.dirname(os.path.realpath(__file__))
    for config in build_config:
        invoke_build_script = "from " + config["build_script"] + \
            " import build_benchmark_source; build_benchmark_source(r'" + \
            config["source_dir"] + "', r'" + \
            config["output_dir"] + "')"
        subprocess.call(['python', '-c', invoke_build_script])

def run_benchmark(benchmark):
    data_dir = os.path.join(BENCHMARKS_DIR, benchmark, DATA_DIR)
    for language in get_benchmark_languages(benchmark):
        print(language)
        executable = os.path.join(BUILD_DIR, benchmark, language, 'benchmark.exe')
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
