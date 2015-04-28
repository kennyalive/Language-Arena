import os

def _get_compiled_runnable(output_dir):
    return [os.path.join(output_dir, 'benchmark.exe')]

def _get_python_runnable(output_dir):
    return ['pypy', os.path.join(output_dir, 'benchmark.py')]

LANGUAGES = [
    {
        'name': 'lang_cpp',
        'runnable_func': _get_compiled_runnable
    },
    {
        'name': 'lang_d',
        'runnable_func': _get_compiled_runnable
    },
    {
        'name': 'lang_go',
        'runnable_func': _get_compiled_runnable
    },
    {
        'name': 'lang_python',
        'runnable_func': _get_python_runnable
    }
]
