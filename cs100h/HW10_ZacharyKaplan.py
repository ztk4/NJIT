#!/usr/bin/python3

from collections import defaultdict
import string

##Zachary Kaplan
##CS 100 2015F Section H03
##HW 8, Oct 16, 2015

#Exercise 4.27
#method fcopy
def fcopy(src, dst):
    with open(src, 'r') as fin, open(dst, 'w') as fout:
        fout.write(fin.read())

#test code for fcopy
def prob1():
    fcopy('example.txt', 'output.txt')
    with open('output.txt', 'r') as fin:
        print(fin.read())

#Exercise 4.29
#method stats
def stats(f):
    with open(f, 'r') as fin:
        lcount, wcount, ccount = 0, 0, 0

        for line in fin:
            lcount += 1
            wcount += len(line.split())
            ccount += len(line)

    print('line count:', lcount)
    print('word count:', wcount)
    print('character count:', ccount)

#test code for stats
def prob2():
    stats('example.txt')

#Exercise from PDF
#method repeatWords
def repeatWords(src, dst):
    with open(src, 'r') as fin, open(dst, 'w') as fout:
        for line in fin:
            words = defaultdict(int)
            for word in line.lower().split():
                words[word.strip(string.punctuation)] += 1

            repeats = [word for word, occ in words.items() if occ > 1]
            if len(repeats):
                print(*repeats, file=fout)

def prob3():
    repeatWords('example.txt', 'output.txt')
    with open('output.txt', 'r') as fin:
        print(fin.read())

#Main Routine
if __name__ == '__main__':

	#grabs global dictionary (everything on file scope)
	glob = globals()

	#loops over each example to be executed
	for prob in range(1, 4):
		#prints the title
		print("Problem #%d:" % prob)

		#executes the proper method by constructing its name in a string,
		#and then using that as a key to the globals dictionary
		glob['prob%d' % prob]()

		#print a new line
		print()
