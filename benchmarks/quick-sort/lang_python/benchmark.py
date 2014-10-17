import sys
import os
import time

def quick_sort(list, left, right):
    if left >= right:
        return
    store_index = left
    for i in range(left, right):
        if list[i] <= list[right]:
            list[i], list[store_index] = list[store_index], list[i]
            store_index += 1
    list[right], list[store_index] = list[store_index], list[right]
    quick_sort(list, left, store_index - 1)
    quick_sort(list, store_index + 1, right)

def validate_result(result, answers_file_name):
     answer = [int(line.strip()) for line in open(answers_file_name)]
     return result == answer

if __name__ == '__main__':
    # prepare input data
    path = os.path.join(sys.argv[1], 'random_numbers')
    numbers = [int(line.strip()) for line in open(path)]

    # run benchmark
    start = time.clock()
    quick_sort(numbers, 0, len(numbers) - 1)
    elapsed_time = int((time.clock() - start) * 1000)

    # return benchmark results
    exit_code = elapsed_time
    if sys.argv[-1] == 'validate':
        answers_file_name = os.path.join(sys.argv[1] , 'sorted_random_numbers')
        exit_code = 0 if validate_result(numbers, answers_file_name) else -1
    exit(exit_code)
