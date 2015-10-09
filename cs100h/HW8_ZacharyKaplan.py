#!/usr/bin/python3

import re

##Zachary Kaplan
##CS 100 2015F Section H03
##HW 8, Oct 16, 2015

#Utility Methods

#Print List Item
#Takes any number of args and keyword args and passes them to print
#while also printing out (a) (b) ... (z) before each line
def printLi(*args, **kwargs):
	#global memory storage for the next letter (basically like static in c)
	global let

	#if a keyword argument 'let' exists, use its value to set let,
	#and then delete the entry
	if 'let' in kwargs:
		let = kwargs['let'];
		del kwargs['let'];

	#print the letter followed by the original args and kwargs passed in
	print('('+let+')', *args, **kwargs);

	#increment the letter
	let = chr(ord(let)+1);

#Exercise 1
#method twoWords
def twoWords(length, firstLetter):
    while True:
        word1 = input("A %d-letter word please " % length)
        if len(word1) == length:
            break;
    while True:
        word2 = input("A word beginning with %s please " %  firstLetter)
        if len(word2) and word2[0].lower() == firstLetter.lower():
            break

    return word1, word2

#test code
def prob1():
    print(twoWords(4, 'B'))

#Exercise 2
#method twoWordsV2
def twoWordsV2(length, firstLetter):
    word1 = ''
    while len(word1) != length:
        word1 = input("A %d-letter word please " % length)

    word2 = ''
    while not len(word2) and word2[0].lower() == firstLetter.lower():
        word2 = input("A word beginning with %s please ", firstLetter)

    return word1, word2

#test code
def prob2():
    print(twoWordsV2(4, 'B'))

def prob3():
    pass

def prob4():
    pass

#Exercise 5
#method enterNewPassword
def enterNewPassword():
    def checklen(passwd):
        return 8 <= len(passwd) <= 15
    def checknum(passwd):
        return re.match(r'.*\d.*', passwd) != None #matches to any number of characters (including 0) followed by at least on digit and any number of characters

    passwd = ''
    while not checklen(passwd) or not checknum(passwd):
        passwd = input("Enter a password: ")
        if not checklen(passwd):
            print("Password must be no less than 8 characters and no more than 15 characters")
        if not checknum(passwd):
            print("Password must contain at least one digit")

#test code
def prob5():
    enterNewPassword()

#Main Routine
if __name__ == '__main__':

	#grabs global dictionary (everything on file scope)
	glob = globals()

	#loops over each example to be executed
	for prob in range(1, 6):
		#prints the title
		print("Problem #%d:" % prob)

		#executes the proper method by constructing its name in a string,
		#and then using that as a key to the globals dictionary
		glob['prob%d' % prob]()

		#print a new line
		print()
