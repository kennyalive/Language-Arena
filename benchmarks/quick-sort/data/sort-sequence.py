import sys

if len(sys.argv) < 2:
    print("usage: sort-sequence path-to-file-with-numbers")
    sys.exit()

path = sys.argv[1]
numbers = [int(line.strip()) for line in open(path)]
numbers.sort()

for number in numbers:
    print(number)
