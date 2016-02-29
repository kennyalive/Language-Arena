import common
import importlib
import registry
import os
import shutil
import subprocess
import sys
import time

from collections import defaultdict

sys.path.append(os.path.join(common.PROJECT_ROOT_PATH))
import config

EQUAL_PERFORMANCE_EPSILON = 3.0 # in percents

COLOR_READY = '\033[92m'
COLOR_NOT_FOUND = '\033[91m'
COLOR_NOTE = '\033[95m'
COLOR_SUMMARY = '\033[93m'
COLOR_DEFAULT = '\033[0m'

FRAMEWORK_DIR = 'framework'
BENCHMARKS_DIR = 'benchmarks'
BUILD_DIR = 'build'
DATA_DIR = 'data'

BENCHMARKS_PATH = os.path.join(common.PROJECT_ROOT_PATH, BENCHMARKS_DIR)
BUILD_PATH = os.path.join(common.PROJECT_ROOT_PATH, BUILD_DIR)


def get_compiler_version(compiler):
    unknown_version = 'unknown compiler version'

    language = None
    for language_configuration in registry.languages:
        for build_configuration in language_configuration['build_configurations']:
            if build_configuration['compiler'] == compiler:
                language = language_configuration['language']
                break

    if language is None:
        return unknown_version

    language_module = importlib.import_module(language)
    get_version_func_name =  'get_' + compiler + '_version'
    get_version = getattr(language_module, get_version_func_name, None)

    if get_version is None:
        return unknown_version

    version_string = get_version(config.compilers[compiler])
    if not version_string:
        return unknown_version

    return version_string


def check_available_compilers():
    show_config_notice = False

    for compiler, path in list(config.compilers.items()):
        print('----------------------------------------')
        print('| compiler: {}'.format(compiler))

        if os.path.exists(path):
            print('| verinfo : {}'.format(get_compiler_version(compiler)))
            print('| status  : ' + COLOR_READY + 'READY' + COLOR_DEFAULT)
        else:
            print('| status  : ' + COLOR_NOT_FOUND + 'NOT FOUND' + COLOR_DEFAULT)
            del config.compilers[compiler]
            show_config_notice = True
    print('----------------------------------------')

    if not config.compilers:
        print('\nNo compilers found!\nUpdate config.py by specifying paths to the installed compilers')
        sys.exit()
    elif show_config_notice:
        print(COLOR_NOTE + 'NOTE' + COLOR_DEFAULT + ': compiler paths can be specified in config.py script')


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
    benchmarks = sorted(os.listdir(BENCHMARKS_PATH))
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
    directories = os.listdir(os.path.join(BENCHMARKS_PATH, benchmark))
    return sorted([dir for dir in directories if dir.startswith('lang_')])


def is_simple_benchmark(benchmark):
    tag_file = os.path.join(BENCHMARKS_PATH, benchmark, 'simple')
    return os.path.exists(tag_file)


def get_language_configuration(language):
    return next((c for c in registry.languages if c['language'] == language), None)


def get_language_display_name(language):
    return get_language_configuration(language)['display_name']


def get_active_build_configurations(language_configuration):
    for build_configuration in language_configuration['build_configurations']:
        compiler = build_configuration['compiler']
        if config.compilers.get(compiler) is not None:
            yield build_configuration


def build_benchmark_with_configuration(benchmark, language, build_configuration):
    language_module = importlib.import_module(language)
    builder_func_name = build_configuration['builder']

    if builder_func_name not in dir(language_module):
        print('failed to find builder function: ' + builder_func_name)
        sys.exit()

    compiler = build_configuration['compiler']
    compiler_path = config.compilers.get(compiler)
    if compiler_path is None:
        print('unknown compiler name: ' + compiler)

    output_dir = os.path.join(BUILD_PATH, benchmark, language, compiler)
    language_dir = os.path.join(BENCHMARKS_PATH, benchmark, language)

    build_launcher_script = (
        "from " + language +
        " import " + builder_func_name + "\n" +
        builder_func_name + 
        "(r'" + language_dir + "', " + 
        "r'" + output_dir + "', " +
        "r'" + compiler_path + "')"
    )

    os.makedirs(output_dir)
    sys.stdout.flush()
    exit_code = subprocess.call(['python', '-c', build_launcher_script])
    if exit_code != 0:
        sys.exit()

    executable = os.path.join(output_dir, common.EXECUTABLE_NAME)
    if not os.path.exists(executable):
        print('failed to build benchmark {} with compiler {}'.format(benchmark, compiler))
        sys.exit()

def build_benchmark(benchmark):
    os.environ['PYTHONPATH'] = FRAMEWORK_DIR
    for language in get_benchmark_languages(benchmark):
        language_configuration = get_language_configuration(language)
        if language_configuration is None:
            print('configuration for {0} is not specified in config.py'.format(language))
            continue
        for build_configuration in get_active_build_configurations(language_configuration):
            build_benchmark_with_configuration(benchmark, language, build_configuration)


def run_benchmark(benchmark, scorecard):
    print('------ Running ' + benchmark + ' ------')
    scorecard.on_benchmark_start(benchmark)
    data_dir = os.path.join(BENCHMARKS_PATH, benchmark, DATA_DIR)

    for language in get_benchmark_languages(benchmark):
        language_configuration = get_language_configuration(language)
        if language_configuration is None:
            continue

        language_best_time = sys.float_info.max

        for build_configuration in get_active_build_configurations(language_configuration):
            print(language + '/' + build_configuration['compiler'])

            output_dir = os.path.join(BUILD_PATH, benchmark, language, build_configuration['compiler'])
            executable = os.path.join(output_dir, common.EXECUTABLE_NAME)

            sys.stdout.flush()
            exit_code = subprocess.call([executable, data_dir])

            if exit_code > 0: # validation failure or runtime error
                sys.exit(exit_code)

            benchmark_result = 0
            try:
                with open(os.path.join(output_dir, 'timing')) as f:
                    content = f.readline()
                    benchmark_result = int(content)
            except OSError:
                print('failed to read benchmark timing')
                sys.exit(1)

            elapsed_time = benchmark_result / 1000.0
            print("{:.3f}".format(elapsed_time))

            compiler = build_configuration.get('name')
            if compiler is None:
                compiler = build_configuration['compiler']

            scorecard.register_benchmark_time(language, compiler, elapsed_time)

    scorecard.on_benchmark_end()


class Scorecard:
    def __init__(self):
        self.scores = defaultdict(int)
        self.language_relative_times = defaultdict(float)
        self.compiler_relative_times = defaultdict(float)

    def on_benchmark_start(self, benchmark):
        self.language_times = {}
        self.compiler_times = {}
        self.points = [10, 5] if is_simple_benchmark(benchmark) else [20, 10]

    def register_benchmark_time(self, language, compiler, time):
        language_best_time = self.language_times.get(language, sys.float_info.max)
        self.language_times[language] = min(language_best_time, time)
        self.compiler_times[compiler] = time

    def on_benchmark_end(self):
        if not self.language_times or not self.points:
            return

        # update language relative times
        sorted_language_times = sorted(self.language_times.items(), key=lambda x: x[1])
        language_normalization_coeff = 1.0 / sorted_language_times[0][1]
        for (language, time) in sorted_language_times:
            self.language_relative_times[language] += time * language_normalization_coeff

        # update compiler relative times
        sorted_compiler_times = sorted(self.compiler_times.items(), key=lambda x: x[1])
        compiler_normalization_coeff = 1.0 / sorted_compiler_times[0][1]
        for (compiler, time) in sorted_compiler_times:
            self.compiler_relative_times[compiler] += time * compiler_normalization_coeff

        # update scores
        cur_place_index = 0
        cur_place_time = 0.0
        prev_earned_points = 0

        print('')
        for i, (language, time) in enumerate(sorted_language_times):
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
                get_language_display_name(language),
                earned_points,
                time * language_normalization_coeff))
        print('')

        self.language_times = None
        self.compiler_times = None
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

        print('========================================')
        print(' Benchmark results')
        print('========================================')

        # print language relative times
        if len(self.language_relative_times) > 1:
            sorted_language_relative_times = \
                sorted(self.language_relative_times.items(), key=lambda x: x[1])
            language_normalization_coeff = 1.0 / sorted_language_relative_times[0][1]

            print('Language relative times:')
            for (language, relative_time) in sorted_language_relative_times:
                language_name = get_language_display_name(language)
                normalized_time = relative_time * language_normalization_coeff
                print('{:3} {:.2f}'.format(language_name, normalized_time))

        # print compiler relative times
        if len(self.compiler_relative_times) > 1:
            sorted_compiler_relative_times = \
                sorted(self.compiler_relative_times.items(), key=lambda x: x[1])
            compiler_normalization_coeff = 1.0 / sorted_compiler_relative_times[0][1]

            print('\nCompiler relative times:')
            for (compiler, relative_time) in sorted_compiler_relative_times:
                normalized_time = relative_time * compiler_normalization_coeff
                print('{:5} {:.2f}'.format(compiler, normalized_time))

        # print final scores
        print('\n' + COLOR_SUMMARY + 'Summary' + COLOR_DEFAULT + ':')
        for i, (score, languages) in enumerate(final_results):
            languages_str = ', '.join(map(lambda x: get_language_display_name(x), languages))
            if i == 0:
                winner_suffix = ' DOMINATES!'
                # special case #1: no winner detected
                if len(languages) > 1:
                    winner_suffix = '. THE BORING DRAW, THE BORING UNIVERSE...'
                # special case #2: only single language was benchmarked
                elif len(self.scores) == 1:
                    winner_suffix = '. YOU CAN\'T DOMINATE WHEN YOU ARE ALONE'
                print('Place 1 [{:2} points]. {}{}'.format(score, languages_str, winner_suffix))
            else:
                print('Place {} [{:2} points]. {}'.format(i+1, score, languages_str))


# DigitalWhip main
if __name__ == '__main__':
    print('Checking available compilers...')
    check_available_compilers()
    print('')

    options = get_options()
    benchmarks = get_benchmarks()

    if not options['skip_build']:
        print('Building benchmarks source code...')
        if os.path.exists(BUILD_PATH):
            shutil.rmtree(BUILD_PATH)
            os.makedirs(BUILD_PATH)
        for benchmark in benchmarks:
            build_benchmark(benchmark)
        print('')

    scorecard = Scorecard()
    for benchmark in benchmarks:
        run_benchmark(benchmark, scorecard)
    scorecard.print_summary()
