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

def is_simple_benchmark(benchmark):
    tag_file = os.path.join(BENCHMARKS_DIR, benchmark, 'simple')
    return os.path.exists(tag_file)

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

def run_benchmark(benchmark, scores_table):
    data_dir = os.path.join(BENCHMARKS_DIR, benchmark, DATA_DIR)

    timing = []
    for language in get_benchmark_languages(benchmark):
        language_best_time = sys.float_info.max

        for builder in benchmark_builders.get(language, []):
            print(language + '/' + builder['name'])
            executable = os.path.join(BUILD_DIR, benchmark, language,
                builder['name'], 'benchmark.exe')
            benchmark_result = subprocess.call([executable, data_dir])
            elapsed_time = benchmark_result / 1000.0
            language_best_time = min(language_best_time, elapsed_time)
            print("{:.3f}".format(elapsed_time))

        timing.append((language, language_best_time))

    simple = is_simple_benchmark(benchmark)
    first_place_score = 10 if simple else 20
    second_place_score = 5 if simple else 10

    timing.sort(key = lambda x: x[1])
    if len(timing) > 0:
        first_language = timing[0][0]
        scores_table[first_language] = scores_table.get(first_language, 0) + first_place_score
        print('First place: {0}. +{1} points'.format(first_language, first_place_score))
    if len(timing) > 1:
        second_language = timing[1][0]
        scores_table[second_language] = scores_table.get(second_language, 0) + second_place_score
        print('Second place: {0}. +{1} points'.format(second_language, second_place_score))

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

    scores_table = {}

    print("")
    for benchmark in benchmarks:
        print('------ Running ' + benchmark + ' ------')
        run_benchmark(benchmark, scores_table)

    sorted_scores_table = sorted(scores_table.items(), key=lambda x: x[1], reverse=True)
    print('')
    for i, score_info in enumerate(sorted_scores_table):
        print('Place ' + str(i + 1) + '. ' + score_info[0] + ' with ' + str(score_info[1]) + ' points')
