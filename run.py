import importlib
import os
import shutil
import subprocess
import sys
import time

from collections import defaultdict

import config

BUILD_DIR = 'build'
BENCHMARKS_DIR = 'benchmarks'
DATA_DIR = 'data'

class Scorecard:
    def __init__(self):
        self.results = defaultdict(lambda: {'score': 0, 'time': 0.0})

    def on_benchmark_start(self, benchmark):
        self.benchmark_timings = {}

        simple_benchmark = is_simple_benchmark(benchmark)
        self.points = [10, 5] if simple_benchmark else [20, 10]

    def register_benchmark_time(self, language, time):
        self.benchmark_timings[language] = time

    def on_benchmark_end(self):
        if not self.benchmark_timings:
            return

        sorted_benchmark_timings = sorted(self.benchmark_timings.items(), key=lambda x: x[1])

        time_scale = 1.0 / sorted_benchmark_timings[0][1]
        comparison_lang_str = ''
        comparison_time_str = ''

        for i, (language, time) in enumerate(sorted_benchmark_timings):
            earned_points = self.points[i] if i < len(self.points) else 0

            result = self.results[language]
            result['score'] += earned_points
            result['time'] += time

            if i < 2:
                position = 'First' if i == 0 else 'Second'
                print('{0} place: {1}. +{2} points'.format(position, language, earned_points))
        
            comparison_lang_str += language
            comparison_time_str +=  '{:.2f}'.format(time * time_scale)
            if i != len(sorted_benchmark_timings) - 1:
                comparison_lang_str += ' : '
                comparison_time_str += ' : '

        print('Relative timing: {0} = {1}'.format(comparison_lang_str, comparison_time_str))

    def print_summary(self):
        sorted_results = sorted(self.results.items(), key=lambda x: x[1]['score'], reverse=True)
        print('')
        print('Summary:')
        for i, (language, result) in enumerate(sorted_results):
            print('Place ' + str(i + 1) + '. ' + language + ' with ' + str(result['score']) + ' points')

def get_benchmark_languages(benchmark):
    directories = os.listdir(os.path.join(BENCHMARKS_DIR, benchmark))
    return sorted([dir for dir in directories if dir.startswith('lang_')])

def is_simple_benchmark(benchmark):
    tag_file = os.path.join(BENCHMARKS_DIR, benchmark, 'simple')
    return os.path.exists(tag_file)

def get_language_configuration(language):
    return next((c for c in config.languages if c['language'] == language), None)

def build_benchmark_with_configuration(benchmark, language, build_configuration):
    language_module = importlib.import_module('scripts.' + language)
    builder_func_name = build_configuration['builder']

    if builder_func_name not in dir(language_module):
        print('failed to find builder function: ' + builder_func_name)
        sys.exit()

    output_dir = os.path.join(BUILD_DIR, benchmark, language, build_configuration['name'])
    output_dir_abs = os.path.abspath(output_dir)

    language_dir = os.path.join(BENCHMARKS_DIR, benchmark, language)
    build_launcher_script = (
        "from scripts." + language +
        " import " + builder_func_name + "\n" +
        builder_func_name + "(r'" +
        language_dir + "', r'" +
        output_dir_abs + "'," +
        "eval(r'''" + repr(build_configuration) + "'''))"
    )

    os.makedirs(output_dir_abs)
    subprocess.call(['python', '-c', build_launcher_script])

def build_benchmark(benchmark):
    os.environ['PYTHONPATH'] = os.path.dirname(os.path.realpath(__file__))
    for language in get_benchmark_languages(benchmark):
        language_configuration = get_language_configuration(language)
        if language_configuration is None:
            print('configuration for {0} is not specified in config.py'.format(language))
            continue
        for build_configuration in language_configuration['build_configurations']:
            build_benchmark_with_configuration(benchmark, language, build_configuration)

def run_benchmark(benchmark, scorecard):
    print('------ Running ' + benchmark + ' ------')
    scorecard.on_benchmark_start(benchmark)
    data_dir = os.path.join(BENCHMARKS_DIR, benchmark, DATA_DIR)

    for language in get_benchmark_languages(benchmark):
        language_configuration = get_language_configuration(language)
        if language_configuration is None:
            continue

        language_best_time = sys.float_info.max

        for build_configuration in language_configuration['build_configurations']:
            print(language + '/' + build_configuration['name'])
            executable = os.path.join(BUILD_DIR, benchmark, language, build_configuration['name'], 'benchmark.exe')
            benchmark_result = subprocess.call([executable, data_dir])
            elapsed_time = benchmark_result / 1000.0
            print("{:.3f}".format(elapsed_time))
            language_best_time = min(language_best_time, elapsed_time)
        scorecard.register_benchmark_time(language, language_best_time)
    scorecard.on_benchmark_end()

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

    scorecard = Scorecard()
    print("")

    for benchmark in benchmarks:
        run_benchmark(benchmark, scorecard)

    scorecard.print_summary()

