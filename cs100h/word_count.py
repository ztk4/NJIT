#!/usr/bin/python3

from collections import defaultdict
import sys, string

if len(sys.argv) != 2:
    print("Please supply only an argument of the file to parse")
    sys.exit(0)

count = defaultdict(int)
total = 0
with open(sys.argv[1], 'r') as f:
    for word in f.read().split():
        count[word.strip(string.punctuation).lower()] += 1
        total += 1

top25 = sorted(count.items(), key=lambda x: x[1], reverse=True)[:25]

print(*['%s\t%d\t%.5f%%' % (word, count, count/total * 100) for word, count in top25], sep='\n', end='\n\n')
