import random

numbers = []

for i in range(0, 100):
  numbers.append(random.gauss(64, 20))

numbers.sort()

for i in range(0, 100):
  print("{} {}".format(i + 1, numbers[i]))
