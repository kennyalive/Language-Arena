import random
N = 4000000

with open("random_numbers", 'w+b') as f:
    f.write(N.to_bytes(4, byteorder='little'))
    for i in range(N):
        number = random.randrange(N)
        f.write(number.to_bytes(4, byteorder='little'))
