import importlib
import os
import shutil
import subprocess
import sys
import time

from config import LANGUAGES

BUILD_DIR = 'build'
BENCHMARKS_DIR = 'benchmarks'
DATA_DIR = 'data'
BUILD_SCRIPT = 'build.py'

def build_benchmark(benchmark):
	# create build configuration for each available language
    build_config = []
    for language in LANGUAGES:
        language_dir = os.path.join(BENCHMARKS_DIR, benchmark, language['name'])
        if not os.path.exists(language_dir):
            continue

        build_script = os.path.join(language_dir, BUILD_SCRIPT)

        output_dir = os.path.join(BUILD_DIR, benchmark, language['name'])
        output_dir = os.path.abspath(output_dir)

        # check for build script alongside the source code
        if os.path.exists(build_script):
            os.makedirs(output_dir)
            build_config.append({
                'build_script': build_script,
                'output_dir': output_dir
            })
        # try to find default build routine
        else:
            language_script_module = importlib.import_module('scripts.' + language['name'])
            if 'default_build' in dir(language_script_module):
                os.makedirs(output_dir)
                build_config.append({
                    'build_script': 'scripts.' + language['name'],
                    'output_dir': output_dir,
                    'source_dir': language_dir
                    })
    # do build for all languages
    os.environ['PYTHONPATH'] = os.path.dirname(os.path.realpath(__file__))
    for config in build_config:
        if (config['build_script'].endswith(BUILD_SCRIPT)):
            subprocess.call(['python', config['build_script'], config['output_dir']])
        else:
            default_build_runner = "from " + config["build_script"] + " import default_build; default_build(r'" + \
                config["source_dir"] + "', r'" + config["output_dir"] + "')"
            subprocess.call(['python', '-c', default_build_runner])

def run_benchmark(benchmark, validate_results):
    data_dir = os.path.join(BENCHMARKS_DIR, benchmark, DATA_DIR)
    for language in LANGUAGES:
        output_dir = os.path.join(BUILD_DIR, benchmark, language['name'])
        if os.path.exists(output_dir):
            # prepare command line to launch benchmark executable
            runnable_command_line = language['runnable_func'](output_dir)
            runnable_command_line.append(data_dir)
            if validate_results:
                runnable_command_line.append('validate')
            # launch benchmark
            print(language['name'])
            start = time.clock()
            benchmark_result = subprocess.call(runnable_command_line)
            # print benchmark results
            if validate_results:
                print('Passed' if benchmark_result == 0 else 'Failed')
            else:
                elapsed_time_benchmark = benchmark_result / 1000.0
                elapsed_time_total = time.clock() - start
                print("benchmark {:.3f}".format(elapsed_time_benchmark))
                print("total {:.3f}\n".format(elapsed_time_total))

if __name__ == '__main__':
    validate_results = len(sys.argv) > 1 and sys.argv[1] == 'validate'

    if os.path.exists(BUILD_DIR):
        shutil.rmtree(BUILD_DIR)
    os.makedirs(BUILD_DIR)

    benchmarks = os.listdir('benchmarks')
    for benchmark in benchmarks:
        build_benchmark(benchmark)

    print("")
    for benchmark in benchmarks:
        print('------ Running ' + benchmark + ' ------')
        run_benchmark(benchmark, validate_results)
