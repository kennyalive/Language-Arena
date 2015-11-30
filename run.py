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

EQUAL_PERFORMANCE_EPSILON = 3.0 # in percents

def get_options():
    options = {
        'skip_build' : False
    }
    for opt in [opt for opt in sys.argv[1:] if opt.startswith('--')]:
        if opt == '--no-build':
            options['skip_build'] = True
        else:
            print('unknown option ' + opt)
            sys.exit()
    return options

def get_benchmarks():
    benchmarks = sorted(os.listdir('benchmarks'))
    selected_benchmarks = [b for b in sys.argv[1:] if not b.startswith('--')]
    if selected_benchmarks:
        for b in selected_benchmarks:
            if not b in benchmarks:
                print('unknown benchmark' + b)
                sys.exit()
        benchmarks = selected_benchmarks
    else:
        simple_benchmarks = sorted([b for b in benchmarks if is_simple_benchmark(b)])
        complex_benchmarks = sorted([b for b in benchmarks if not is_simple_benchmark(b)])
        benchmarks = simple_benchmarks + complex_benchmarks
    return benchmarks

def get_benchmark_languages(benchmark):
    directories = os.listdir(os.path.join(BENCHMARKS_DIR, benchmark))
    return sorted([dir for dir in directories if dir.startswith('lang_')])

def is_simple_benchmark(benchmark):
    tag_file = os.path.join(BENCHMARKS_DIR, benchmark, 'simple')
    return os.path.exists(tag_file)

def get_language_configuration(language):
    return next((c for c in config.languages if c['language'] == language), None)

def get_language_display_name(language):
    return get_language_configuration(language)['display_name']

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
    exit_code = subprocess.call(['python', '-c', build_launcher_script])
    if exit_code != 0:
        sys.exit()

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

            if benchmark_result < 0: # validation failure or runtime error
                sys.exit(benchmark_result)

            elapsed_time = benchmark_result / 1000.0
            print("{:.3f}".format(elapsed_time))
            language_best_time = min(language_best_time, elapsed_time)
        scorecard.register_benchmark_time(language, language_best_time)
    scorecard.on_benchmark_end()

class Scorecard:
    def __init__(self):
        self.scores = defaultdict(int)

    def on_benchmark_start(self, benchmark):
        self.benchmark_timings = {}
        self.points = [10, 5] if is_simple_benchmark(benchmark) else [20, 10]

    def register_benchmark_time(self, language, time):
        self.benchmark_timings[language] = time

    def on_benchmark_end(self):
        if not self.benchmark_timings or not self.points:
            return

        sorted_benchmark_timings = sorted(self.benchmark_timings.items(), key=lambda x: x[1])
        time_scale = 1.0 / sorted_benchmark_timings[0][1]

        cur_place_index = 0
        cur_place_time = 0.0
        prev_earned_points = 0

        print('')
        for i, (language, time) in enumerate(sorted_benchmark_timings):
            if i == 0:
                cur_place_time = time
            else:
                performance_difference = (time - cur_place_time) / cur_place_time * 100
                if performance_difference > EQUAL_PERFORMANCE_EPSILON:
                    cur_place_index += 1
                    cur_place_time = time

            earned_points = self.points[cur_place_index] if cur_place_index < len(self.points) else 0
            if i >= 2 and earned_points != prev_earned_points:
                earned_points = 0

            prev_earned_points = earned_points
            self.scores[language] += earned_points

            print('{:3} earned {:2} points, relative time {:.2f}'.format(
                get_language_configuration(language)['display_name'], earned_points, time*time_scale))
        print('')

        self.benchmark_timings = None
        self.points = None

    def print_summary(self):
        # group languages with the same scores
        sorted_scores = sorted(self.scores.items(), key=lambda x: x[1], reverse=True)
        final_results = []
        prev_score = 0
        for i, (language, score) in enumerate(sorted_scores):
            if i > 0 and score == prev_score:
                final_results[-1][1].append(language)
            else:
                final_results.append((score, [language]))
            prev_score = score

        # print final benchmark results
        print('Summary:')
        for i, (score, languages) in enumerate(final_results):
            languages_str = ', '.join(map(lambda x: get_language_display_name(x), languages))
            if i == 0:
                winner_suffix = ' DOMINATES!' if len(languages) == 1 else '. THE BORING DRAW, THE BORING UNIVERSE...'
                print('Place 1 [{:2} points]. {}{}'.format(score, languages_str, winner_suffix))
            else:
                print('Place {} [{:2} points]. {}'.format(i+1, score, languages_str))

# DigitalWhip main
if __name__ == '__main__':
    options = get_options()
    benchmarks = get_benchmarks()

    if not options['skip_build']:
        if os.path.exists(BUILD_DIR):
            shutil.rmtree(BUILD_DIR)
            os.makedirs(BUILD_DIR)
        for benchmark in benchmarks:
            build_benchmark(benchmark)

    scorecard = Scorecard()
    for benchmark in benchmarks:
        run_benchmark(benchmark, scorecard)
    scorecard.print_summary()
