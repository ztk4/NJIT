print(sum([i for i in range(1000) if not i%3 or not i%5]))

print(sum(set([i for i in range(3, 1000, 3)] + [i for i in range(5, 1000, 5)])))

print(sum([i for i in range(3, 1000, 3)] + [i for i in range(5, 1000, 5) if i%3]))
